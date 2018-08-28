open Webapi.Dom;

type event = START_BATCHING | STOP_BATCHING;

open Time;

let intent = (e: event) =>
  switch e {
  | START_BATCHING => "START_BATCHING"
  | STOP_BATCHING => "STOP_BATCHING"
  }

module Batch = {
  let startBatching = () => {
    let ev = CustomEvent.make(intent(START_BATCHING));
    Window.dispatchEvent(ev, window) |> ignore
  }

  let stopBatching = () => {
    let ev = CustomEvent.make(intent(STOP_BATCHING));
    Window.dispatchEvent(ev, window) |> ignore
  }

  let batchOnElapsed = time => {
    if(Overtime.check(time)) {
      startBatching();
    } 
  }
}