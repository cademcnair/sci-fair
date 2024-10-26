const { readFileSync, writeFileSync } = require("node:fs")
const content = readFileSync("./wordle.txt").toString("utf-8").split("\n")
const letters = [{}, {}, {}, {}, {}]
content.forEach(i => i.split("").forEach((ii, d) => {
  if (letters[d][ii]) ++letters[d][ii]; else letters[d][ii] = 1
}))
letters.forEach((i, d) => i["_"] = Object.entries(i).reduce((a,b)=>a+b[1], 0))
letters.forEach((i, d) => writeFileSync(`./starting/${d+1}.txt`, Object.entries(i).sort().map(i => `${i[0]} ${i[1]}`).join("\n")))