// This file is part of Blend2D project <https://blend2d.com>
//
// See LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifdef BLEND2D_APPS_ENABLE_SKIA

#include "app.h"
#include "module_plutovg.h"
#include <plutovg/plutovg.h>

namespace blbench {

// static inline SkIRect toSkIRect(const BLRectI& rect) {
//   return SkIRect::MakeXYWH(rect.x, rect.y, rect.w, rect.h);
// }

// static inline SkRect toSkRect(const BLRect& rect) {
//   return SkRect::MakeXYWH(SkScalar(rect.x), SkScalar(rect.y), SkScalar(rect.w), SkScalar(rect.h));
// }

static uint32_t toSkBlendMode(BLCompOp compOp) {
  switch (compOp) {
    case BL_COMP_OP_SRC_OVER   : return uint32_t();
    case BL_COMP_OP_SRC_COPY   : return uint32_t();
    case BL_COMP_OP_SRC_IN     : return uint32_t();
    case BL_COMP_OP_SRC_OUT    : return uint32_t();
    case BL_COMP_OP_SRC_ATOP   : return uint32_t();
    case BL_COMP_OP_DST_OVER   : return uint32_t();
    case BL_COMP_OP_DST_COPY   : return uint32_t();
    case BL_COMP_OP_DST_IN     : return uint32_t();
    case BL_COMP_OP_DST_OUT    : return uint32_t();
    case BL_COMP_OP_DST_ATOP   : return uint32_t();
    case BL_COMP_OP_XOR        : return uint32_t();
    case BL_COMP_OP_CLEAR      : return uint32_t();
    case BL_COMP_OP_PLUS       : return uint32_t();
    case BL_COMP_OP_MODULATE   : return uint32_t();
    case BL_COMP_OP_MULTIPLY   : return uint32_t();
    case BL_COMP_OP_SCREEN     : return uint32_t();
    case BL_COMP_OP_OVERLAY    : return uint32_t();
    case BL_COMP_OP_DARKEN     : return uint32_t();
    case BL_COMP_OP_LIGHTEN    : return uint32_t();
    case BL_COMP_OP_COLOR_DODGE: return uint32_t();
    case BL_COMP_OP_COLOR_BURN : return uint32_t();
    case BL_COMP_OP_HARD_LIGHT : return uint32_t();
    case BL_COMP_OP_SOFT_LIGHT : return uint32_t();
    case BL_COMP_OP_DIFFERENCE : return uint32_t();
    case BL_COMP_OP_EXCLUSION  : return uint32_t();

    default: return 0xFFFFFFFFu;
  }
}

struct PlutovgModule : public BenchModule {
  SkCanvas* _skCanvas {};
  SkBitmap _skSurface;
  SkBitmap _skSprites[4];

  SkBlendMode _blendMode {};
  SkTileMode _gradientTileMode {};

  PlutovgModule();
  virtual ~PlutovgModule();
};

PlutovgModule::PlutovgModule() {
  strcpy(_name, "plutovg");
}
PlutovgModule::~PlutovgModule() {}

bool PlutovgModule::supportsCompOp(BLCompOp compOp) const {
  return toSkBlendMode(compOp) != 0xFFFFFFFFu;
}

bool PlutovgModule::supportsStyle(uint32_t style) const {
  return style == kBenchStyleSolid         ||
         style == kBenchStyleLinearPad     ||
         style == kBenchStyleLinearRepeat  ||
         style == kBenchStyleLinearReflect ||
         style == kBenchStyleRadialPad     ||
         style == kBenchStyleRadialRepeat  ||
         style == kBenchStyleRadialReflect ||
         style == kBenchStyleConic         ||
         style == kBenchStylePatternNN     ||
         style == kBenchStylePatternBI     ;
}

void PlutovgModule::onBeforeRun() {
  int w = int(_params.screenW);
  int h = int(_params.screenH);
  uint32_t style = _params.style;

  // Initialize the sprites.
  for (uint32_t i = 0; i < kBenchNumSprites; i++) {
    const BLImage& sprite = _sprites[i];

    BLImageData spriteData;
    sprite.getData(&spriteData);

    SkImageInfo spriteInfo = SkImageInfo::Make(spriteData.size.w, spriteData.size.h, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
    _skSprites[i].installPixels(spriteInfo, spriteData.pixelData, size_t(spriteData.stride));
  }

  // Initialize the surface and the context.
  BLImageData surfaceData;
  _surface.create(w, h, _params.format);
  _surface.makeMutable(&surfaceData);

  SkImageInfo surfaceInfo = SkImageInfo::Make(w, h, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
  _skSurface.installPixels(surfaceInfo, surfaceData.pixelData, size_t(surfaceData.stride));
  _skSurface.erase(0x00000000, SkIRect::MakeXYWH(0, 0, w, h));

  _skCanvas = new SkCanvas(_skSurface);

  // Setup globals.
  _blendMode = SkBlendMode(toSkBlendMode(_params.compOp));
  _gradientTileMode = SkTileMode::kClamp;

  switch (style) {
    case kBenchStyleLinearPad    : _gradientTileMode = SkTileMode::kClamp ; break;
    case kBenchStyleLinearRepeat : _gradientTileMode = SkTileMode::kRepeat; break;
    case kBenchStyleLinearReflect: _gradientTileMode = SkTileMode::kMirror; break;
    case kBenchStyleRadialPad    : _gradientTileMode = SkTileMode::kClamp ; break;
    case kBenchStyleRadialRepeat : _gradientTileMode = SkTileMode::kRepeat; break;
    case kBenchStyleRadialReflect: _gradientTileMode = SkTileMode::kMirror; break;
  }
}

void PlutovgModule::onAfterRun() {
  delete _skCanvas;
  _skCanvas = nullptr;

  _skSurface.reset();
  for (uint32_t i = 0; i < kBenchNumSprites; i++) {
    _skSprites[i].reset();
  }
}

// void PlutovgModule::onDoRectAligned(bool stroke) {
//   BLSizeI bounds(_params.screenW, _params.screenH);
//   uint32_t style = _params.style;
//   int wh = _params.shapeSize;

//   SkPaint p;
//   p.setStyle(stroke ? SkPaint::kStroke_Style : SkPaint::kFill_Style);
//   p.setAntiAlias(true);
//   p.setBlendMode(_blendMode);
//   p.setStrokeWidth(SkScalar(_params.strokeWidth));

//   if (style == kBenchStyleSolid) {
//     for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
//       BLRectI rect = _rndCoord.nextRectI(bounds, wh, wh);

//       p.setColor(_rndColor.nextRgba32().value);
//       _skCanvas->drawIRect(toSkIRect(rect), p);
//     }
//   }
//   else {
//     for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
//       BLRectI rect = _rndCoord.nextRectI(bounds, wh, wh);

//       p.setShader(makeShader(style, rect));
//       _skCanvas->drawIRect(toSkIRect(rect), p);
//     }
//   }
// }

// void PlutovgModule::onDoRectSmooth(bool stroke) {
//   BLSize bounds(_params.screenW, _params.screenH);
//   uint32_t style = _params.style;
//   double wh = _params.shapeSize;

//   SkPaint p;
//   p.setStyle(stroke ? SkPaint::kStroke_Style : SkPaint::kFill_Style);
//   p.setAntiAlias(true);
//   p.setBlendMode(_blendMode);
//   p.setStrokeWidth(SkScalar(_params.strokeWidth));

//   if (style == kBenchStyleSolid) {
//     for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
//       BLRect rect = _rndCoord.nextRect(bounds, wh, wh);

//       p.setColor(_rndColor.nextRgba32().value);
//       _skCanvas->drawRect(toSkRect(rect), p);
//     }
//   }
//   else {
//     for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
//       BLRect rect = _rndCoord.nextRect(bounds, wh, wh);

//       p.setShader(makeShader(style, rect));
//       _skCanvas->drawRect(toSkRect(rect), p);
//     }
//   }
// }

// void PlutovgModule::onDoRectRotated(bool stroke) {
//   BLSize bounds(_params.screenW, _params.screenH);
//   uint32_t style = _params.style;

//   double cx = double(_params.screenW) * 0.5;
//   double cy = double(_params.screenH) * 0.5;
//   double wh = _params.shapeSize;
//   double angle = 0.0;

//   SkPaint p;
//   p.setStyle(stroke ? SkPaint::kStroke_Style : SkPaint::kFill_Style);
//   p.setAntiAlias(true);
//   p.setBlendMode(_blendMode);
//   p.setStrokeWidth(SkScalar(_params.strokeWidth));

//   for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
//     BLRect rect = _rndCoord.nextRect(bounds, wh, wh);

//     _skCanvas->rotate(SkRadiansToDegrees(angle), SkScalar(cx), SkScalar(cy));

//     if (style == kBenchStyleSolid)
//       p.setColor(_rndColor.nextRgba32().value);
//     else
//       p.setShader(makeShader(style, rect));

//     _skCanvas->drawRect(toSkRect(rect), p);
//     _skCanvas->resetMatrix();
//   }
// }

// void PlutovgModule::onDoRoundSmooth(bool stroke) {
//   BLSize bounds(_params.screenW, _params.screenH);
//   uint32_t style = _params.style;
//   double wh = _params.shapeSize;

//   SkPaint p;
//   p.setStyle(stroke ? SkPaint::kStroke_Style : SkPaint::kFill_Style);
//   p.setAntiAlias(true);
//   p.setBlendMode(_blendMode);
//   p.setStrokeWidth(SkScalar(_params.strokeWidth));

//   for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
//     BLRect rect = _rndCoord.nextRect(bounds, wh, wh);
//     double radius = _rndExtra.nextDouble(4.0, 40.0);

//     if (style == kBenchStyleSolid)
//       p.setColor(_rndColor.nextRgba32().value);
//     else
//       p.setShader(makeShader(style, rect));

//     _skCanvas->drawRoundRect(toSkRect(rect), SkScalar(radius), SkScalar(radius), p);
//   }
// }

// void PlutovgModule::onDoRoundRotated(bool stroke) {
//   BLSize bounds(_params.screenW, _params.screenH);
//   uint32_t style = _params.style;

//   double cx = double(_params.screenW) * 0.5;
//   double cy = double(_params.screenH) * 0.5;
//   double wh = _params.shapeSize;
//   double angle = 0.0;

//   SkPaint p;
//   p.setStyle(stroke ? SkPaint::kStroke_Style : SkPaint::kFill_Style);
//   p.setAntiAlias(true);
//   p.setBlendMode(_blendMode);
//   p.setStrokeWidth(SkScalar(_params.strokeWidth));

//   for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
//     _skCanvas->rotate(SkRadiansToDegrees(angle), SkScalar(cx), SkScalar(cy));

//     BLRect rect = _rndCoord.nextRect(bounds, wh, wh);
//     double radius = _rndExtra.nextDouble(4.0, 40.0);

//     if (style == kBenchStyleSolid)
//       p.setColor(_rndColor.nextRgba32().value);
//     else
//       p.setShader(makeShader(style, rect));

//     _skCanvas->drawRoundRect(toSkRect(rect), SkScalar(radius), SkScalar(radius), p);
//     _skCanvas->resetMatrix();
//   }
// }

// void PlutovgModule::onDoPolygon(uint32_t mode, uint32_t complexity) {
//   BLSizeI bounds(_params.screenW - _params.shapeSize,
//                  _params.screenH - _params.shapeSize);
//   uint32_t style = _params.style;
//   double wh = double(_params.shapeSize);

//   enum { kPointCapacity = 128 };
//   if (complexity > kPointCapacity)
//     return;

//   SkPoint points[kPointCapacity];
//   bool stroke = (mode == 2);

//   SkPaint p;
//   p.setStyle(stroke ? SkPaint::kStroke_Style : SkPaint::kFill_Style);
//   p.setAntiAlias(true);
//   p.setBlendMode(_blendMode);
//   p.setStrokeWidth(SkScalar(_params.strokeWidth));

//   // SKIA cannot draw a polygon without having a path, so we have two cases here.
//   if (!stroke) {
//     for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
//       BLPoint base(_rndCoord.nextPoint(bounds));

//       SkPath path;
//       path.setFillType(mode != 0 ? SkPathFillType::kEvenOdd : SkPathFillType::kWinding);

//       double x, y;
//       x = _rndCoord.nextDouble(base.x, base.x + wh);
//       y = _rndCoord.nextDouble(base.y, base.y + wh);
//       path.moveTo(SkPoint::Make(SkScalar(x), SkScalar(y)));

//       for (uint32_t j = 1; j < complexity; j++) {
//         x = _rndCoord.nextDouble(base.x, base.x + wh);
//         y = _rndCoord.nextDouble(base.y, base.y + wh);
//         path.lineTo(SkPoint::Make(SkScalar(x), SkScalar(y)));
//       }

//       if (style == kBenchStyleSolid) {
//         p.setColor(_rndColor.nextRgba32().value);
//       }
//       else {
//         BLRect rect(base.x, base.y, wh, wh);
//         p.setShader(makeShader(style, rect));
//       }

//       _skCanvas->drawPath(path, p);
//     }
//   }
//   else {
//     for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
//       BLPoint base(_rndCoord.nextPoint(bounds));

//       for (uint32_t j = 0; j < complexity; j++) {
//         double x = _rndCoord.nextDouble(base.x, base.x + wh);
//         double y = _rndCoord.nextDouble(base.y, base.y + wh);
//         points[j].set(SkScalar(x), SkScalar(y));
//       }

//       if (style == kBenchStyleSolid) {
//         p.setColor(_rndColor.nextRgba32().value);
//       }
//       else {
//         BLRect rect(base.x, base.y, wh, wh);
//         p.setShader(makeShader(style, rect));
//       }

//       _skCanvas->drawPoints(SkCanvas::kPolygon_PointMode, complexity, points, p);
//     }
//   }
// }

// void PlutovgModule::onDoShape(bool stroke, const BLPoint* pts, size_t count) {
//   BLSizeI bounds(_params.screenW - _params.shapeSize,
//                  _params.screenH - _params.shapeSize);
//   uint32_t style = _params.style;
//   double wh = double(_params.shapeSize);

//   SkPath path;
//   bool start = true;

//   for (size_t i = 0; i < count; i++) {
//     double x = pts[i].x;
//     double y = pts[i].y;

//     if (x == -1.0 && y == -1.0) {
//       start = true;
//       continue;
//     }

//     if (start) {
//       path.moveTo(SkScalar(x * wh), SkScalar(y * wh));
//       start = false;
//     }
//     else {
//       path.lineTo(SkScalar(x * wh), SkScalar(y * wh));
//     }
//   }

//   SkPaint p;
//   p.setStyle(stroke ? SkPaint::kStroke_Style : SkPaint::kFill_Style);
//   p.setAntiAlias(true);
//   p.setBlendMode(_blendMode);
//   p.setStrokeWidth(SkScalar(_params.strokeWidth));

//   for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
//     BLPoint base(_rndCoord.nextPoint(bounds));

//     _skCanvas->translate(SkScalar(base.x), SkScalar(base.y));

//     if (style == kBenchStyleSolid) {
//       p.setColor(_rndColor.nextRgba32().value);
//     }
//     else {
//       BLRect rect(base.x, base.y, wh, wh);
//       p.setShader(makeShader(style, rect));
//     }

//     _skCanvas->drawPath(path, p);
//     _skCanvas->resetMatrix();
//   }
// }

// BenchModule* createPlutovgModule() {
//   return new PlutovgModule();
// }

} // {blbench}

#endif // BLEND2D_APPS_ENABLE_SKIA
