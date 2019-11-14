const Module = require("../out/fontconverter.js");
const wasm = Module({
  arguments: ["rRobotoMono-Regular.ttf", "5"],
  noInitialRun: true
});
const func = wasm.cwrap("convert", "string", [
  "string",
  "number",
  "number",
  "number",
  "number"
]);

wasm.onRuntimeInitialized = function() {
  console.log("JSON Output:");
  console.log(func("monaco.ttf", 5, 1));

  console.log("C++ Output:");
  console.log(func("5RobotoMono-Regular.ttf", 5, 0));
};
