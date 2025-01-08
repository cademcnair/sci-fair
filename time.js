#! /usr/bin/env node
const rl = require("readline").createInterface({
  output: process.stdout,
  input: process.stdin
});
const { writeFile, copyFile, rm } = require("fs/promises");
const { exec } = require("child_process")
(async()=>{
  await writeFile("./mode.c++", `
    const int MODE = 3; // 1
    const int GEN_THREADS = 1; // 25
    const int MODEL_THREADS = 1; // 2
  `)
  let num = await new Promise((r) => rl.question("Number of models: ", r))
  for (let i = 1; i <= num; ++i) {
    let output = ``, total = 0
    for (let ii = 0; ii < 10; ++ii) {
      await copyFile(`time-models/model-1-${i}`, `models/model-1-1`)
      let start = Date.now()
      await new Promise((r) => exec(`echo "0 1" | ./train`))
      let seconds = ((Date.now() - start)/1000)/2308
      output += `${ii} ${seconds}\n`
      total += seconds
      await rm(`models/model-1-1`)
    }
    total /= 10
    output += `∑ ${total}`
    await writeFile(`time-results/model-${i}`, output)
  }
  rl.close()
})()