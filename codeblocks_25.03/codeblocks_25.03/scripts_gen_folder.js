const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

/* ---------- PNG writer ---------- */
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
  function chunk(type, data) {
    const len = Buffer.alloc(4);
    len.writeUInt32BE(data.length, 0);
    const t = Buffer.from(type);
    const crcBuf = Buffer.alloc(4);
    crcBuf.writeUInt32BE(crc32(Buffer.concat([t, data])), 0);
    return Buffer.concat([len, t, data, crcBuf]);
  }
  const ihdr = Buffer.alloc(13);
  ihdr.writeUInt32BE(width, 0);
  ihdr.writeUInt32BE(height, 4);
  ihdr[8] = 8; ihdr[9] = 6; ihdr[10] = 0; ihdr[11] = 0; ihdr[12] = 0;
  const raw = Buffer.alloc((width * 4 + 1) * height);
  for (let y = 0; y < height; y++) {
    raw[y * (width * 4 + 1)] = 0;
    for (let x = 0; x < width; x++) {
      const p = pixels[y][x];
      const i = y * (width * 4 + 1) + 1 + x * 4;
      raw[i] = p[0]; raw[i + 1] = p[1]; raw[i + 2] = p[2]; raw[i + 3] = p[3];
    }
  }
  return Buffer.concat([
    Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]),
    chunk('IHDR', ihdr),
    chunk('IDAT', zlib.deflateSync(raw, { level: 9 })),
    chunk('IEND', Buffer.alloc(0))
  ]);
}

/* ---------- tiny anti-aliased rasterizer (supersampled) ---------- */
const SS = 8; // supersample factor

function makeCanvas(w, h) {
  const W = w * SS, H = h * SS;
  const buf = new Float32Array(W * H * 4); // premultiplied RGBA 0..1
  function blend(i, r, g, b, a) {
    const dr = buf[i], dg = buf[i + 1], db = buf[i + 2], da = buf[i + 3];
    const na = a + da * (1 - a);
    if (na <= 0) { buf[i] = buf[i+1] = buf[i+2] = buf[i+3] = 0; return; }
    buf[i]     = (r * a + dr * da * (1 - a)) / na;
    buf[i + 1] = (g * a + dg * da * (1 - a)) / na;
    buf[i + 2] = (b * a + db * da * (1 - a)) / na;
    buf[i + 3] = na;
  }
  // rounded rect in *icon* coordinates (0..w, 0..h)
  function roundRect(x, y, rw, rh, r, color) {
    const [cr, cg, cb, ca] = color;
    const x0 = x * SS, y0 = y * SS, x1 = (x + rw) * SS, y1 = (y + rh) * SS;
    const rr = r * SS;
    for (let py = Math.max(0, Math.floor(y0)); py < Math.min(H, Math.ceil(y1)); py++) {
      for (let px = Math.max(0, Math.floor(x0)); px < Math.min(W, Math.ceil(x1)); px++) {
        const cx = px + 0.5, cy = py + 0.5;
        const qx = Math.max(x0 + rr - cx, 0, cx - (x1 - rr));
        const qy = Math.max(y0 + rr - cy, 0, cy - (y1 - rr));
        if (qx * qx + qy * qy <= rr * rr) {
          blend((py * W + px) * 4, cr, cg, cb, ca);
        }
      }
    }
  }
  function gradientRoundRect(x, y, rw, rh, r, cTop, cBottom) {
    for (let py = Math.floor(y * SS); py < Math.ceil((y + rh) * SS); py++) {
      const t = (py - y * SS) / (rh * SS);
      const c = [
        cTop[0] + (cBottom[0] - cTop[0]) * t,
        cTop[1] + (cBottom[1] - cTop[1]) * t,
        cTop[2] + (cBottom[2] - cTop[2]) * t,
        cTop[3] + (cBottom[3] - cTop[3]) * t
      ];
      // draw one-pixel-high rounded rect slice
      roundRect(x, py / SS, rw, 1 / SS, r, c);
    }
  }
  function toPixels() {
    const out = [];
    for (let y = 0; y < h; y++) {
      const row = [];
      for (let x = 0; x < w; x++) {
        let r = 0, g = 0, b = 0, a = 0;
        for (let sy = 0; sy < SS; sy++) {
          for (let sx = 0; sx < SS; sx++) {
            const i = (((y * SS + sy) * W) + (x * SS + sx)) * 4;
            r += buf[i] * buf[i + 3];
            g += buf[i + 1] * buf[i + 3];
            b += buf[i + 2] * buf[i + 3];
            a += buf[i + 3];
          }
        }
        const n = SS * SS;
        if (a > 0.0001) row.push([Math.round(r / a * 255), Math.round(g / a * 255), Math.round(b / a * 255), Math.round(a / n * 255)]);
        else row.push([0, 0, 0, 0]);
      }
      out.push(row);
    }
    return out;
  }
  return { roundRect, gradientRoundRect, toPixels };
}

/* ---------- modern folder icon ---------- */
function drawFolder(size) {
  const c = makeCanvas(size, size);
  const s = size / 22; // design grid is 22x22

  // palette
  const backDark = [21, 101, 192, 1];    // #1565C0
  const frontTop = [66, 165, 245, 1];    // #42A5F5
  const frontBot = [25, 118, 210, 1];    // #1976D2
  const rim = [13, 71, 161, 1];          // #0D47A1

  // tab (back part of the folder)
  c.roundRect(1.5 * s, 3.2 * s, 8.5 * s, 5.5 * s, 1.6 * s, backDark);
  // back body
  c.roundRect(1.5 * s, 6.2 * s, 19 * s, 13.2 * s, 2.4 * s, backDark);
  // front body with vertical gradient
  c.gradientRoundRect(1.5 * s, 8.6 * s, 19 * s, 10.8 * s, 2.4 * s, frontTop, frontBot);
  // subtle darker rim at the bottom for depth
  c.gradientRoundRect(1.5 * s, 17.2 * s, 19 * s, 2.2 * s, 2.4 * s, [30, 136, 229, 0.55], [21, 101, 192, 0.65]);
  // top edge highlight
  c.gradientRoundRect(2.2 * s, 8.9 * s, 17.6 * s, 1.1 * s, 0.55 * s, [255, 255, 255, 0.42], [255, 255, 255, 0.08]);

  return createPng(size, size, c.toPixels());
}

const dir = path.join('src', 'src', 'resources', 'images');
fs.writeFileSync(path.join(dir, 'folder.png'), drawFolder(22));
console.log('wrote folder.png 22x22');
