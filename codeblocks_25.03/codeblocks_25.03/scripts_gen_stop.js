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

/* ---------- anti-aliased rasterizer (supersampled) ---------- */
const SS = 8;

function makeCanvas(w, h) {
  const W = w * SS, H = h * SS;
  const buf = new Float32Array(W * H * 4);
  function blend(i, r, g, b, a) {
    const dr = buf[i], dg = buf[i + 1], db = buf[i + 2], da = buf[i + 3];
    const na = a + da * (1 - a);
    if (na <= 0) { buf[i] = buf[i+1] = buf[i+2] = buf[i+3] = 0; return; }
    buf[i]     = (r * a + dr * da * (1 - a)) / na;
    buf[i + 1] = (g * a + dg * da * (1 - a)) / na;
    buf[i + 2] = (b * a + db * da * (1 - a)) / na;
    buf[i + 3] = na;
  }
  function roundRect(x, y, rw, rh, r, color) {
    const [cr, cg, cb, ca] = color;
    const x0 = x * SS, y0 = y * SS, x1 = (x + rw) * SS, y1 = (y + rh) * SS;
    const rr = Math.max(0, Math.min(r * SS, (x1 - x0) / 2, (y1 - y0) / 2));
    for (let py = Math.max(0, Math.floor(y0)); py < Math.min(H, Math.ceil(y1)); py++) {
      for (let px = Math.max(0, Math.floor(x0)); px < Math.min(W, Math.ceil(x1)); px++) {
        const cx = px + 0.5, cy = py + 0.5;
        const qx = Math.max(x0 + rr - cx, 0, cx - (x1 - rr));
        const qy = Math.max(y0 + rr - cy, 0, cy - (y1 - rr));
        if (qx * qx + qy * qy <= rr * rr) blend((py * W + px) * 4, cr, cg, cb, ca);
      }
    }
  }
  function gradientRoundRect(x, y, rw, rh, r, cTop, cBottom) {
    const steps = Math.ceil(rh * SS);
    for (let i = 0; i < steps; i++) {
      const py = y * SS + i;
      if (py < 0 || py >= H) continue;
      const t = i / Math.max(1, steps - 1);
      const c = [
        cTop[0] + (cBottom[0] - cTop[0]) * t,
        cTop[1] + (cBottom[1] - cTop[1]) * t,
        cTop[2] + (cBottom[2] - cTop[2]) * t,
        cTop[3] + (cBottom[3] - cTop[3]) * t
      ];
      roundRect(x, py / SS, rw, 1.02 / SS, r, c);
    }
  }
  function line(x1, y1, x2, y2, color) {
    const [cr, cg, cb, ca] = color;
    const sx = x1 * SS, sy = y1 * SS, ex = x2 * SS, ey = y2 * SS;
    const dx = Math.abs(ex - sx), dy = Math.abs(ey - sx === 0 ? ey - sy : ey - sy);
    // proper Bresenham-ish with anti-aliasing via supersampling done by looping over thick line
    const thick = 2.2 * SS;
    const steps = Math.max(Math.abs(ex - sx), Math.abs(ey - sy), 1);
    for (let i = 0; i <= steps; i++) {
      const t = i / steps;
      const cx = sx + (ex - sx) * t;
      const cy = sy + (ey - sy) * t;
      const half = thick / 2;
      for (let oy = Math.max(0, Math.floor(cy - half)); oy < Math.min(H, Math.ceil(cy + half)); oy++) {
        for (let ox = Math.max(0, Math.floor(cx - half)); ox < Math.min(W, Math.ceil(cx + half)); ox++) {
          const d = Math.hypot(ox + 0.5 - cx, oy + 0.5 - cy);
          if (d <= half) blend((oy * W + ox) * 4, cr, cg, cb, ca);
        }
      }
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
  return { roundRect, gradientRoundRect, line, toPixels };
}

/* ---------- modern stop icon ----------
 * Red filled rounded square + white X in the middle.
 * Clean, modern, unmistakable "stop" metaphor.
 */
function drawStop(size) {
  const c = makeCanvas(size, size);
  const s = size / 22;

  // red rounded square background
  c.gradientRoundRect(1.5 * s, 1.5 * s, 19 * s, 19 * s, 4.0 * s,
    [239, 68, 68, 1],   // #EF4444 top
    [185, 28, 28, 1]    // #B91C1C bottom
  );

  // white X centered
  const pad = 5.0 * s;
  c.line(pad, pad, size - pad, size - pad, [255, 255, 255, 1]);
  c.line(pad, size - pad, size - pad, pad, [255, 255, 255, 1]);

  return createPng(size, size, c.toPixels());
}

const dir = path.join('src', 'src', 'resources', 'images');
fs.writeFileSync(path.join(dir, 'stop.png'), drawStop(22));
console.log('wrote stop.png 22x22');
