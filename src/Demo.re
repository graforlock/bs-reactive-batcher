open Webapi.Dom;
open Batcher;
open Postman;
open Time;

type batch = { key: string, payload: Dom.event };
type event = START_BATCHING | STOP_BATCHING;

type bucket = { id: string, bucket: Js.Dict.t(array(Dom.event)), mutable size: int, time: int };

let postmanClient: postman = Sockets.start(~url="localhost:8000");
let windowTarget = Window.asEventTarget(window);

let uwrapEventTarget = 
  fun
  | Some(v) => Element.asEventTarget (v)
  | None => raise(Invalid_argument("Passed none to unwrap"));

let uniqueId = () => string_of_float(Js.Math.random()) |> Js.String.substring(~from=2, ~to_=16);

let incrA = document |> Document.querySelector("#incrA") |> uwrapEventTarget;
let incrB = document |> Document.querySelector("#incrB") |> uwrapEventTarget;

let incrAStream = Most.(
  fromEvent("click", incrA, false)
  |> map((e: Dom.event) => { key: "click-A", payload: e })
);

let incrBStream = Most.(
  fromEvent("click", incrB, false)
  |> map((e: Dom.event) => { key: "click-B", payload: e })
);

let mousemoveStream = Most.(
  fromEvent("mousemove", windowTarget, false)
  |> map((e: Dom.event) => { key: "mousemove", payload: e })
)

let modules = Most.(mergeArray([| mousemoveStream, incrAStream, incrBStream |]));
let start = Most.(fromEvent("START_BATCHING", windowTarget, false));

let startLoop = Most.(
  timestamp(start)
  |> map((stamp) => scan(
    (acc, {key, payload}) => {
      switch (Js.Dict.get(acc.bucket, key)) {
      | Some(xs) => Js.Dict.set(acc.bucket, key, Array.append([| payload |], xs))
      | None => Js.Dict.set(acc.bucket, key, [| payload |])
      }
      acc.size = acc.size + 1;
      acc
    },
    { id: uniqueId(), size: 0, bucket: Js.Dict.empty(), time: stamp##time },
    modules
  ))
);

let main = Most.(
  switchLatest(startLoop)
  |> combine((timer, bucket) => (timer, bucket), repeatedly)
  |> filter(((_, {size, time})) => {
      switch size {
      | 0 => Batch.batchOnElapsed(time); false
      | n when (n mod 100 == 0) => true
      | _ when Overtime.check(time) => Batch.batchOnElapsed(time); true
      | _ => false
      };
    })
  |> skipRepeatsWith(((_, next), (_, prev)) => prev.id == next.id)
);


Most.(
  main |> observe(((_, currentBatch)) => {
    Js.log(currentBatch);
    Batch.startBatching();
    postmanClient.sendBatch();
  })
);

Batch.startBatching();