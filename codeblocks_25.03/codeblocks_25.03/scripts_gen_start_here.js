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
  ihdr[8] = 8; // bit depth
  ihdr[9] = 6; // color type RGBA
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

function drawNew(sizeW, sizeH) {
  const pixels = Array(sizeH).fill().map(()=>Array(sizeW).fill([0,0,0,0]));
  function set(x,y,r,g,b,a=255){ if(x>=0&&x<sizeW&&y>=0&&y<sizeH) pixels[y][x]=[r,g,b,a]; }
  function rect(x1,y1,x2,y2,r,g,b,a=255){ for(let y=Math.max(0,y1);y<=Math.min(sizeH-1,y2);y++) for(let x=Math.max(0,x1);x<=Math.min(sizeW-1,x2);x++) set(x,y,r,g,b,a); }
  const border = [60,60,60,255];
  const paper = [250,250,252,255];
  const blue = [0,122,204,255];
  // document
  rect(6,4, sizeW-9, sizeH-6, border[0],border[1],border[2],255);
  rect(7,5, sizeW-10, sizeH-7, paper[0],paper[1],paper[2],255);
  // folded corner
  const cx = sizeW-9, cy = 4;
  rect(cx-3,4,cx,7, border[0],border[1],border[2],255);
  rect(cx-2,5,cx-1,6, paper[0],paper[1],paper[2],255);
  // plus symbol
  const px = Math.floor(sizeW*0.72), py = Math.floor(sizeH*0.72);
  rect(px-1, py-4, px, py+3, blue[0],blue[1],blue[2],255);
  rect(px-4, py-1, px+3, py, blue[0],blue[1],blue[2],255);
  return createPng(sizeW, sizeH, pixels);
}

function drawOpen(sizeW, sizeH) {
  const pixels = Array(sizeH).fill().map(()=>Array(sizeW).fill([0,0,0,0]));
  function set(x,y,r,g,b,a=255){ if(x>=0&&x<sizeW&&y>=0&&y<sizeH) pixels[y][x]=[r,g,b,a]; }
  function rect(x1,y1,x2,y2,r,g,b,a=255){ for(let y=Math.max(0,y1);y<=Math.min(sizeH-1,y2);y++) for(let x=Math.max(0,x1);x<=Math.min(sizeW-1,x2);x++) set(x,y,r,g,b,a); }
  const border = [60,60,60,255];
  const folderBack = [160,120,60,255];
  const folderFront = [200,160,90,255];
  const blue = [0,122,204,255];
  // folder back
  rect(5,12, sizeW-6, sizeH-7, border[0],border[1],border[2],255);
  rect(6,13, sizeW-7, sizeH-8, folderBack[0],folderBack[1],folderBack[2],255);
  // folder tab
  rect(5,8, Math.floor(sizeW/2)+2, 13, border[0],border[1],border[2],255);
  rect(6,9, Math.floor(sizeW/2)+1, 12, folderBack[0],folderBack[1],folderBack[2],255);
  // folder front
  rect(4,15, sizeW-5, sizeH-6, border[0],border[1],border[2],255);
  rect(5,16, sizeW-6, sizeH-7, folderFront[0],folderFront[1],folderFront[2],255);
  // arrow down into folder (suggestive)
  const ax = Math.floor(sizeW*0.7), ay = Math.floor(sizeH*0.55);
  rect(ax-1, ay-4, ax, ay+1, blue[0],blue[1],blue[2],255);
  // arrow head
  rect(ax-2, ay, ax+1, ay, blue[0],blue[1],blue[2],255);
  rect(ax-1, ay+1, ax, ay+1, blue[0],blue[1],blue[2],255);
  return createPng(sizeW, sizeH, pixels);
}

const dir = path.join('src','src','resources','start_here');
fs.writeFileSync(path.join(dir, 'new.png'), drawNew(47, 42));
console.log('wrote new.png 47x42');
fs.writeFileSync(path.join(dir, 'open.png'), drawOpen(47, 47));
console.log('wrote open.png 47x47');
console.log('done');
