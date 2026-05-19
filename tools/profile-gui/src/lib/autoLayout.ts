export interface LayoutItem {
  key: string;
  w: number;
  h: number;
}

export interface LayoutResult {
  positions: Map<string, { x: number; y: number; page: number }>;
  pageCount: number;
}

export function guillotineLayout(
  items: LayoutItem[],
  screenW: number,
  screenH: number,
  gap = 4
): LayoutResult {
  const sorted = [...items].sort((a, b) => b.w * b.h - a.w * a.h);

  type Rect = { x: number; y: number; w: number; h: number };
  const pages: Rect[][] = [[{ x: 0, y: 0, w: screenW, h: screenH }]];
  const positions = new Map<string, { x: number; y: number; page: number }>();

  for (const item of sorted) {
    const iw = item.w + gap;
    const ih = item.h + gap;
    let placed = false;

    for (let p = 0; p < pages.length; p++) {
      const rects = pages[p];
      let bestIdx = -1;
      let bestShort = Infinity;

      for (let r = 0; r < rects.length; r++) {
        if (rects[r].w >= iw && rects[r].h >= ih) {
          const shortSide = Math.min(rects[r].w - iw, rects[r].h - ih);
          if (shortSide < bestShort) {
            bestShort = shortSide;
            bestIdx = r;
          }
        }
      }

      if (bestIdx >= 0) {
        const rect = rects[bestIdx];
        positions.set(item.key, { x: rect.x, y: rect.y, page: p });
        rects.splice(bestIdx, 1);
        if (rect.w - iw > 0) rects.push({ x: rect.x + iw, y: rect.y, w: rect.w - iw, h: ih });
        if (rect.h - ih > 0) rects.push({ x: rect.x, y: rect.y + ih, w: rect.w, h: rect.h - ih });
        placed = true;
        break;
      }
    }

    if (!placed) {
      const p = pages.length;
      pages.push([{ x: 0, y: 0, w: screenW, h: screenH }]);
      const rect = pages[p][0];
      positions.set(item.key, { x: 0, y: 0, page: p });
      pages[p].splice(0, 1);
      if (rect.w - iw > 0) pages[p].push({ x: iw, y: 0, w: rect.w - iw, h: ih });
      if (rect.h - ih > 0) pages[p].push({ x: 0, y: ih, w: rect.w, h: rect.h - ih });
    }
  }

  return { positions, pageCount: pages.length };
}
