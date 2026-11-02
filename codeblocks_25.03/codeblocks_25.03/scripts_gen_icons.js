const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

function createPng(width, height, pixels) {
  // pixels: array of [r,g,b,a] or object per row
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
    const crcBuf = Buffer.alloc(4);
    const crc = crc32(Buffer.concat([typeBuf, data]));
    crcBuf.writeUInt32BE(crc, 0);
    return Buffer.concat([len, typeBuf, data, crcBuf]);
  }
  // IHDR
  const ihdr = Buffer.alloc(13);
  ihdr.writeUInt32BE(width, 0);
  ihdr.writeUInt32BE(height, 4);
  ihdr[8] = 8; // bit depth
  ihdr[9] = 6; // color type RGBA
  ihdr[10] = 0; // compression
  ihdr[11] = 0; // filter
  ihdr[12] = 0; // interlace
  // IDAT
  let raw = Buffer.alloc((width * 4 + 1) * height);
  for (let y = 0; y < height; y++) {
    raw[y * (width * 4 + 1)] = 0; // filter type 0
    for (let x = 0; x < width; x++) {
      const p = pixels[y][x] || [0,0,0,0];
      const idx = y * (width * 4 + 1) + 1 + x * 4;
      raw[idx] = p[0]; raw[idx+1] = p[1]; raw[idx+2] = p[2]; raw[idx+3] = p[3];
    }
  }
  const compressed = zlib.deflateSync(raw);
  const png = Buffer.concat([
    Buffer.from([137,80,78,71,13,10,26,10]),
    makeChunk('IHDR', ihdr),
    makeChunk('IDAT', compressed),
    makeChunk('IEND', Buffer.alloc(0))
  ]);
  return png;
}

// Simple modern icon drawing
function drawIcon(name, size) {
  const pixels = Array(size).fill().map(()=>Array(size).fill([0,0,0,0]));
  function set(x,y,r,g,b,a=255){ if(x>=0&&x<size&&y>=0&&y<size) pixels[y][x]=[r,g,b,a]; }
  function rect(x1,y1,x2,y2,r,g,b,a=255){ for(let y=y1;y<=y2;y++) for(let x=x1;x<=x2;x++) set(x,y,r,g,b,a); }
  function line(x1,y1,x2,y2,r,g,b,a=255){ 
    // simple bresenham-ish
    const dx=Math.abs(x2-x1), dy=Math.abs(y2-y1);
    const sx=x1<x2?1:-1, sy=y1<y2?1:-1;
    let err=dx-dy;
    while(true){ set(x1,y1,r,g,b,a); if(x1==x2&&y1==y2) break; const e2=2*err; if(e2>-dy){err-=dy;x1+=sx;} if(e2<dx){err+=dx;y1+=sy;} }
  }
  const c = [0, 122, 204, 255]; // blue accent
  const dark = [40,40,40,255];
  const gray = [100,100,100,255];
  if (name === 'filenew') {
    // document with + 
    rect(4,2, Math.floor(size/2)+2, size-4, dark[0],dark[1],dark[2],255);
    rect(5,3, Math.floor(size/2)+1, size-5, 250,250,250,255);
    // fold corner
    const cx = Math.floor(size/2)+2, cy=2;
    rect(cx-2,2,cx,4, dark[0],dark[1],dark[2],255);
    rect(cx-1,3,cx,3, 250,250,250,255);
    // plus
    const px = Math.floor(size*0.7), py = Math.floor(size*0.7);
    rect(px-1, py-3, px, py+2, c[0],c[1],c[2],255);
    rect(px-3, py-1, px+2, py, c[0],c[1],c[2],255);
  } else if (name === 'fileopen') {
    rect(3, size-4, size-4, size-3, dark[0],dark[1],dark[2],255);
    rect(3,4, size-5, size-4, dark[0],dark[1],dark[2],255);
    rect(3,4, 4, size-3, dark[0],dark[1],dark[2],255);
    rect(size-5,6, size-4, size-4, dark[0],dark[1],dark[2],255);
    // fill folder
    for(let y=5;y<size-4;y++){ for(let x=5;x<size-5;x++){ if(y===5||y===size-5||x===5||x===size-5) set(x,y,gray[0],gray[1],gray[2],255); else set(x,y,245,245,245,255); }}
  } else if (name === 'filesave') {
    rect(4,2, size-5, size-3, dark[0],dark[1],dark[2],255);
    rect(5,3, size-6, size-4, 250,250,250,255);
    rect(Math.floor(size/2)-3, 3, Math.floor(size/2)+2, 8, dark[0],dark[1],dark[2],255);
    rect(Math.floor(size/2)-2, 4, Math.floor(size/2)+1, 7, 230,230,230,255);
    rect(7, size-9, size-8, size-5, dark[0],dark[1],dark[2],255);
    rect(8, size-8, size-9, size-6, 220,220,220,255);
  } else if (name === 'editcopy') {
    rect(6,2, size-4, size-6, dark[0],dark[1],dark[2],255);
    rect(7,3, size-5, size-7, 250,250,250,255);
    rect(4,6, size-6, size-4, dark[0],dark[1],dark[2],255);
    rect(5,7, size-7, size-5, 250,250,250,255);
  } else if (name === 'editcut') {
    // scissors simplified
    const cx = Math.floor(size/2), cy = Math.floor(size/2);
    for(let i=-2;i<=2;i++){ set(cx+i, cy-1, dark[0],dark[1],dark[2],255); set(cx+i, cy+1, dark[0],dark[1],dark[2],255); }
    set(cx, cy, dark[0],dark[1],dark[2],255);
    set(cx-3, cy-3, dark[0],dark[1],dark[2],255); set(cx-2, cy-2, dark[0],dark[1],dark[2],255);
    set(cx-3, cy+3, dark[0],dark[1],dark[2],255); set(cx-2, cy+2, dark[0],dark[1],dark[2],255);
  } else if (name === 'editpaste') {
    rect(5,4, size-6, size-5, dark[0],dark[1],dark[2],255);
    rect(6,5, size-7, size-6, 250,250,250,255);
    rect(Math.floor(size/2)-2,2,size-9,5,dark[0],dark[1],dark[2],255);
    rect(Math.floor(size/2)-1,3,size-10,4,230,230,230,255);
  } else if (name === 'undo') {
    line(Math.floor(size/2), Math.floor(size/2)-3, 4, Math.floor(size/2), c[0],c[1],c[2],255);
    line(4, Math.floor(size/2), Math.floor(size/2), Math.floor(size/2)+3, c[0],c[1],c[2],255);
    line(Math.floor(size/2), Math.floor(size/2)-2, 6, Math.floor(size/2), c[0],c[1],c[2],255);
    line(6, Math.floor(size/2), Math.floor(size/2), Math.floor(size/2)+2, c[0],c[1],c[2],255);
  } else if (name === 'redo') {
    line(Math.floor(size/2), Math.floor(size/2)-3, size-5, Math.floor(size/2), c[0],c[1],c[2],255);
    line(size-5, Math.floor(size/2), Math.floor(size/2), Math.floor(size/2)+3, c[0],c[1],c[2],255);
    line(Math.floor(size/2), Math.floor(size/2)-2, size-7, Math.floor(size/2), c[0],c[1],c[2],255);
    line(size-7, Math.floor(size/2), Math.floor(size/2), Math.floor(size/2)+2, c[0],c[1],c[2],255);
  } else if (name === 'searchreplace') {
    // magnifier
    const cx=Math.floor(size/3)+1, cy=Math.floor(size/3)+1;
    for(let y=cy-3;y<=cy+2;y++){ for(let x=cx-3;x<=cx+2;x++){ const d=Math.hypot(x-cx,y-cy); if(d>2.2&&d<3.3) set(x,y,dark[0],dark[1],dark[2],255); }}
    line(cx+2, cy+2, cx+4, cy+4, dark[0],dark[1],dark[2],255);
    // arrows
    line(cx+4, cy+1, size-3, cy+1, c[0],c[1],c[2],255);
    line(size-3, cy+1, size-4, cy, c[0],c[1],c[2],255);
    line(size-3, cy+1, size-4, cy+2, c[0],c[1],c[2],255);
    line(size-3, cy+4, cx+4, cy+4, c[0],c[1],c[2],255);
    line(cx+4, cy+4, cx+5, cy+3, c[0],c[1],c[2],255);
    line(cx+4, cy+4, cx+5, cy+5, c[0],c[1],c[2],255);
  } else {
    // generic box
    rect(4,4,size-5,size-5,dark[0],dark[1],dark[2],255);
    rect(5,5,size-6,size-6,240,240,240,255);
  }
  return createPng(size,size,pixels);
}

const targets = [
  {name:'filenew', sizes:[16,32]},
  {name:'fileopen', sizes:[16,32]},
  {name:'filesave', sizes:[16,32]},
  {name:'editcopy', sizes:[16,32]},
  {name:'editcut', sizes:[16,32]},
  {name:'editpaste', sizes:[16,32]},
  {name:'undo', sizes:[16,32]},
  {name:'redo', sizes:[16,32]},
  {name:'searchreplace', sizes:[16,32]},
];

for (const t of targets) {
  for (const sz of t.sizes) {
    const out = path.join('src','src','resources','images', sz+'x'+sz, t.name + '.png');
    const png = drawIcon(t.name, sz);
    fs.writeFileSync(out, png);
    console.log('wrote', out);
  }
}
console.log('done');
