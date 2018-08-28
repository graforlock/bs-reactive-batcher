type postman = {
  url: string,
  sendBatch: unit => unit,
};

module Sockets = {
  let start = (~url: string) => {
    url: url,
    sendBatch: () => ()
  }
};