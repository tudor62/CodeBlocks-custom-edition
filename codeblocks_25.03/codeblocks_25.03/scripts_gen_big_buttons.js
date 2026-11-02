const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

function createPng(width, height, pixels) {
  function crc32(buf) {
    let crc = 0xffffffff;
    for (let i = 0; i < buf.length; i++) {
      crc ^= buf[i];
      for (let j = 0; j < 8; j++) {
        if (crc & 1) crc = (crc >>> 1) ^ 0xedb88320;
        else crc = crc >>> 1;
      }
    }
    return (crc ^ 0xffffffff) >>> 0;
  }
  function makeChunk(type, data) {
    const len = Buffer.alloc(4);
    len.writeUInt32BE(data.length, 0);
    const typeBuf = Buffer.from(type);
    const crc = crc32(Buffer.concat([typeBuf, data]));
    const crcBuf = Buffer.alloc(4);
    crcBuf.writeUInt32BE(crc, 0);
    return Buffer.concat([len, typeBuf, data, crcBuf]);
  }
  const ihdr = Buffer.alloc(13);
  ihdr.writeUInt32BE(width, 0);
  ihdr.writeUInt32BE(height, 4);
  ihdr[8] = 8;
  ihdr[9] = 6;
  ihdr[10] = 0;
  ihdr[11] = 0;
  ihdr[12] = 0;
  let raw = Buffer.alloc((width * 4 + 1) * height);
  for (let y = 0; y < height; y++) {
    raw[y * (width * 4 + 1)] = 0;
    for (let x = 0; x < width; x++) {
      const p = pixels[y][x] || [0,0,0,0];
      const idx = y * (width * 4 + 1) + 1 + x * 4;
      raw[idx] = p[0]; raw[idx+1] = p[1]; raw[idx+2] = p[2]; raw[idx+3] = p[3];
    }
  }
  const compressed = zlib.deflateSync(raw);
  return Buffer.concat([
    Buffer.from([137,80,78,71,13,10,26,10]),
    makeChunk('IHDR', ihdr),
    makeChunk('IDAT', compressed),
    makeChunk('IEND', Buffer.alloc(0))
  ]);
}

function drawClose(size) {
  const pixels = Array(size).fill().map(()=>Array(size).fill([0,0,0,0]));
  function set(x,y,r,g,b,a=255){ if(x>=0&&x<size&&y>=0&&y<size) pixels[y][x]=[r,g,b,a]; }
  const gray = [90,90,90,255];
  const light = [160,160,160,255];
  // X
  for(let i=2;i<size-2;i++){
    set(i,i, gray[0],gray[1],gray[2],255);
    set(i,size-1-i, gray[0],gray[1],gray[2],255);
    if(i>=3 && i<size-3){
      set(i,i, light[0],light[1],light[2],128);
      set(i,size-1-i, light[0],light[1],light[2],128);
    }
  }
  return createPng(size,size,pixels);
}

function drawCompile(size) {
  const pixels = Array(size).fill().map(()=>Array(size).fill([0,0,0,0]));
  function set(x,y,r,g,b,a=255){ if(x>=0&&x<size&&y>=0&&y<size) pixels[y][x]=[r,g,b,a]; }
  function rect(x1,y1,x2,y2,r,g,b,a=255){ for(let y=Math.max(0,y1);y<=Math.min(size-1,y2);y++) for(let x=Math.max(0,x1);x<=Math.min(size-1,x2);x++) set(x,y,r,g,b,a); }
  const dark = [40,40,40,255];
  const blue = [0,122,204,255];
  // gear-like shape simplified
  rect(9,4,11,7,dark[0],dark[1],dark[2],255);
  rect(4,9,7,11,dark[0],dark[1],dark[2],255);
  rect(14,9,17,11,dark[0],dark[1],dark[2],255);
  rect(9,14,11,17,dark[0],dark[1],dark[2],255);
  rect(5,5,7,7,dark[0],dark[1],dark[2],255);
  rect(14,5,16,7,dark[0],dark[1],dark[2],255);
  rect(5,14,7,16,dark[0],dark[1],dark[2],255);
  rect(14,14,16,16,dark[0],dark[1],dark[2],255);
  // center
  rect(7,7,14,14,blue[0],blue[1],blue[2],255);
  rect(8,8,13,13,20,120,200,255);
  // hole
  rect(10,10,11,11,250,250,250,255);
  return createPng(size,size,pixels);
}

function drawCompileRun(size) {
  const pixels = Array(size).fill().map(()=>Array(size).fill([0,0,0,0]));
  function set(x,y,r,g,b,a=255){ if(x>=0&&x<size&&y>=0&&y<size) pixels[y][x]=[r,g,b,a]; }
  function rect(x1,y1,x2,y2,r,g,b,a=255){ for(let y=Math.max(0,y1);y<=Math.min(size-1,y2);y++) for(let x=Math.max(0,x1);x<=Math.min(size-1,x2);x++) set(x,y,r,g,b,a); }
  const dark = [40,40,40,255];
  const blue = [0,122,204,255];
  const green = [40,160,80,255];
  // gear
  rect(8,4,10,7,dark[0],dark[1],dark[2],255);
  rect(4,8,7,10,dark[0],dark[1],dark[2],255);
  rect(13,8,16,10,dark[0],dark[1],dark[2],255);
  rect(8,13,10,16,dark[0],dark[1],dark[2],255);
  rect(5,5,6,6,dark[0],dark[1],dark[2],255);
  rect(13,5,14,6,dark[0],dark[1],dark[2],255);
  rect(5,13,6,14,dark[0],dark[1],dark[2],255);
  rect(13,13,14,14,dark[0],dark[1],dark[2],255);
  rect(6,6,13,13,blue[0],blue[1],blue[2],255);
  rect(7,7,12,12,20,120,200,255);
  // play triangle
  rect(14,10,14,11,green[0],green[1],green[2],255);
  rect(15,9,15,12,green[0],green[1],green[2],255);
  rect(16,8,16,13,green[0],green[1],green[2],255);
  rect(17,9,17,12,green[0],green[1],green[2],255);
  rect(18,10,18,11,green[0],green[1],green[2],255);
  return createPng(size,size,pixels);
}

const dir = path.join('src','src','resources','images');
fs.writeFileSync(path.join(dir, 'closebutton.png'), drawClose(12));
console.log('wrote closebutton.png 12x12');
fs.writeFileSync(path.join(dir, 'compile.png'), drawCompile(22));
console.log('wrote compile.png 22x22');
fs.writeFileSync(path.join(dir, 'compilerun.png'), drawCompileRun(22));
console.log('wrote compilerun.png 22x22');
console.log('done');
