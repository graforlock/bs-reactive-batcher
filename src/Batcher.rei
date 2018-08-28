module Batch: {
  let startBatching: unit => unit;
  let stopBatching: unit => unit;
  let batchOnElapsed: int => unit; 
}