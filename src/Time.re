
let repeatedly = Most.(timestamp(periodic(100)));

module Overtime = {
  let check = time => int_of_float(Js.Date.now()) - time >= 3000;
}