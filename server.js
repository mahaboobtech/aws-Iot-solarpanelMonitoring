// import section ////
//1
// var awsIot = require("aws-iot-device-sdk");
import awsIot from 'aws-iot-device-sdk'
// const express = require("express");
import express from 'express'
//2
import bodyParser from 'body-parser'
// const bodyParser = require('body-parser');
import dotenv from 'dotenv'
// const dotenv = require('dotenv');
import user from './routes.js'
// var user = require("./routes.js")


// import section end ////




// variable pre required ///////
var clientTokenUpdate;
var gval = "0";
var rval = "1";

var thingShadows = awsIot.thingShadow({
  keyPath: "private123w.pem.key",
  certPath: "certifakevala.pem.crt",
  caPath: "rootCa.pem",
  clientId: "iotj",
  host: "ae3f86adufff-ats.iot.us-east-1.amazonaws.com",
});

///variables pre required end //////////

dotenv.config();

const app = express();

app.use(bodyParser.json())

console.log("Server-side code running");



app.use(express.static("public"));

app.use('/api', user)


app.listen(8080, () => {
  console.log("listening on 8080");
});

app.get("/", (req, res) => {
  res.sendFile(__dirname + "/index.html");
});

// turning off the light for 5 sec 
app.post("/clicked", (req, res) => {
  var thingShadows = awsIot.thingShadow({
    keyPath: "private123w.pem.key",
    certPath: "certifakevala.pem.crt",
    caPath: "rootCa.pem",
    clientId: "iotj",
    host: "ae3f86adufff-ats.iot.us-east-1.amazonaws.com",
  });
  const click = { clickTime: new Date() };
  console.log(click);
  console.log("the button clicked");
  var gval = "0";
  var rval = "1";
  console.log("the ongoing process of aws");
  thingShadows.on("connect", function () {
    //
    // After connecting to the AWS IoT platform, register interest in the
    // Thing Shadow named 'fake1'.
    console.log("the aws is connected");
    //
    thingShadows.register("esp_test8266", {}, function () {
      // Once registration is complete, update the Thing Shadow named
      // 'RGBLedLamp' with the latest device state and save the clientToken
      // so that we can correlate it with status or timeout events.
      //
      // Thing shadow state
      //
      var rgbLedLampState = { state: { desired: { led: gval, led1: rval } } };

      clientTokenUpdate = thingShadows.update("esp_test8266", rgbLedLampState);
      console.log("the data is updated");
      //
      // The update method returns a clientToken; if non-null, this value will
      // be sent in a 'status' event when the operation completes, allowing you
      // to know whether or not the update was successful.  If the update method
      // returns null, it's because another operation is currently in progress and
      // you'll need to wait until it completes (or times out) before updating the
      // shadow.
      //
      if (clientTokenUpdate === null) {
        console.log("update shadow failed, operation still in progress");
      }
    });
  });

  thingShadows.on("status",function (thingName, stat, clientToken, stateObject) {
      console.log(
        "received  s " +
          stat +
          " on " +
          thingName +
          ": " +
          JSON.stringify(stateObject)
      );
      //
      // These events report the status of update(), get(), and delete()
      // calls.  The clientToken value associated with the event will have
      // the same value which was returned in an earlier call to get(),
      // update(), or delete().  Use status events to keep track of the
      // status of shadow operations.
      //
    }
  );

  thingShadows.on("delta", function (thingName, stateObject) {
    console.log(
      "received delta on " + thingName + ": " + JSON.stringify(stateObject)
    );
  });

  thingShadows.on("timeout", function (thingName, clientToken) {
    console.log(
      "received timeout on " + thingName + " with token: " + clientToken
    );
    //
    // In the event that a shadow operation times out, you'll receive
    // one of these events.  The clientToken value associated with the
    // event will have the same value which was returned in an earlier
    // call to get(), update(), or delete().
    //
  });
});

// app.post("/clickedoff", (req, res) => {
//   var thingShadows = awsIot.thingShadow({
//     keyPath: "private123w.pem.key",
//     certPath: "certifakevala.pem.crt",
//     caPath: "rootCa.pem",
//     clientId: "iotj",
//     host: "ae3f86adufff-ats.iot.us-east-1.amazonaws.com",
//   });
//   const click = { clickTime: new Date() };
//   console.log(click);
//   console.log("the button clicked");
//   var gval = "0";
//   var rval = "0";
//   thingShadows.on("connect", function () {
//     //
//     // After connecting to the AWS IoT platform, register interest in the
//     // Thing Shadow named 'fake1'.
//     console.log("the aws is connected");
//     //
//     thingShadows.register("esp_test8266", {}, function () {
//       // Once registration is complete, update the Thing Shadow named
//       // 'RGBLedLamp' with the latest device state and save the clientToken
//       // so that we can correlate it with status or timeout events.
//       //
//       // Thing shadow state
//       //
//       var rgbLedLampState = { state: { desired: { led: gval, led1: rval } } };

//       clientTokenUpdate = thingShadows.update("esp_test8266", rgbLedLampState);
//       console.log("the data is updated");
//       //
//       // The update method returns a clientToken; if non-null, this value will
//       // be sent in a 'status' event when the operation completes, allowing you
//       // to know whether or not the update was successful.  If the update method
//       // returns null, it's because another operation is currently in progress and
//       // you'll need to wait until it completes (or times out) before updating the
//       // shadow.
//       //
//       if (clientTokenUpdate === null) {
//         console.log("update shadow failed, operation still in progress");
//       }
//     });
//   });

//   thingShadows.on(
//     "status",
//     function (thingName, stat, clientToken, stateObject) {
//       console.log(
//         "received  s " +
//           stat +
//           " on " +
//           thingName +
//           ": " +
//           JSON.stringify(stateObject)
//       );
//       //
//       // These events report the status of update(), get(), and delete()
//       // calls.  The clientToken value associated with the event will have
//       // the same value which was returned in an earlier call to get(),
//       // update(), or delete().  Use status events to keep track of the
//       // status of shadow operations.
//       //
//     }
//   );

//   thingShadows.on("delta", function (thingName, stateObject) {
//     console.log(
//       "received delta on " + thingName + ": " + JSON.stringify(stateObject)
//     );
//   });

//   thingShadows.on("timeout", function (thingName, clientToken) {
//     console.log(
//       "received timeout on " + thingName + " with token: " + clientToken
//     );
//     //
//     // In the event that a shadow operation times out, you'll receive
//     // one of these events.  The clientToken value associated with the
//     // event will have the same value which was returned in an earlier
//     // call to get(), update(), or delete().
//     //
//   });
// });
