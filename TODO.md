# Gut Toolkit Roadmap

**Score: 78 done, 2 partial, 12 punted, 17 remaining** out of 109 features.

---

## Layout

- [x] Canvas — absolute positioning with attached Left/Top
- [x] StackPanel — vertical/horizontal auto-stacking
- [x] Grid — rows/columns with Auto/Pixel/Star sizing, spans
- [x] Margin / Padding — on all elements
- [x] Alignment — HorizontalAlignment, VerticalAlignment
- [x] Min/Max constraints — minWidth, maxWidth, minHeight, maxHeight
- [x] WrapPanel — flow layout with line wrapping
- [x] DockPanel — dock children to edges
- [x] ViewBox — scale-to-fit container
- [x] Anchor layout — left+right pins = auto-stretch on resize
- [x] UniformGrid — all cells equal size, automatic row/column count
- [x] FlexPanel — CSS Flexbox-style layout with grow/shrink/basis, justify, align
- [x] RelativePanel — position elements relative to each other or panel edges
- [x] RadialPanel — arrange children in a circle or arc

## Rendering

- [x] Solid color fill
- [x] Rounded rectangles
- [x] Linear gradients (top-to-bottom)
- [x] Borders / stroke — strokeRect, strokeRoundedRect, Panel borderColor/borderWidth
- [x] Drop shadows — soft (blur>0) + hard (blur=0)
- [x] Opacity
- [x] Clip to bounds
- [x] Rounded-rect clipping — SDF fragment shader
- [x] Ellipse — fill + stroke
- [x] Image / texture — drawImage, Image element, stretch modes
- [x] Radial gradients — RadialGradientBrush with multi-stop support, rect/rounded-rect/ellipse rendering
- [x] Inset / inner shadows — drawInsetShadow with per-vertex alpha ring, Panel insetShadow properties
- [x] Path rendering — Path flattening (quad/cubic/arc subdivision), ear-clipping fill, polyline stroke
- [x] Polylines — drawPolyline with miter/bevel/round joins, flat/square/round caps, fillPolygon via ear-clipping
- [x] Transforms — per-element scaleX/scaleY/rotation/skewX/skewY (centre-origin), full affine matrix, inverse hit-test
- [ ] Perspective transforms — 3×3 homogeneous matrix, quad mapping (trapezoid), per-vertex or per-pixel 1/w divide, projective inverse hit-test
- [x] Backdrop blur / frosted glass
- [p] Blend modes — multiply, screen, overlay, etc. (PUNT)
- [x] 9-slice / 9-patch image rendering — drawImageNineSlice, Image::Stretch::NineSlice, sliceBorders property
- [p] Render-to-texture — offscreen render targets (PUNT)

## Text

- [x] Text element — stb_truetype glyph atlas rendering
- [x] Font size
- [x] Font weight / style — FontWeight/FontStyle properties, auto-detection from OS/2+head tables, font registry lookup
- [x] Text alignment — Left, Center, Right applied during render via drawTextMultiline
- [x] Text wrapping — word wrap via TextShaper::shapeMultiline in render path
- [x] Text truncation / ellipsis — CharacterEllipsis + WordEllipsis, single-line
- [~] Multiple font families — font registry with findFont(family, weight, style) best-match lookup; no system font enumeration (also, what about bundled fonts with the app?)
- [p] Text selection (PUNT)
- [p] Rich text / inline formatting — bold, italic, color spans within one block (PUNT)
- [x] Text shadow — textShadowColor/Offset/BlurRadius, hard + soft (multi-pass blur) shadows
- [x] Text outline / stroke — textStrokeColor/Width, 8–16 direction multi-pass outline
- [x] Text glow — textGlowColor/Radius, multi-ring radial spread with alpha falloff

## Input / Interaction

- [x] Mouse enter / leave (hover)
- [x] Mouse click
- [x] Mouse press / release
- [x] Mouse wheel / scroll
- [x] Keyboard input — full KeyEvent with key codes, modifiers, character
- [x] Focus system — tab navigation, focus scope, focus visual
- [x] Hit testing — zIndex-sorted
- [x] isHitTestVisible
- [ ] Mouse drag — capture exists, no general drag-and-drop API
- [x] Cursor styles — Element.cursor property, Context callback, NSCursor mapped on macOS
- [~] Clipboard — TextBox uses NSPasteboard; no general API yet
- [p] IME input — CJK composition support (PUNT)
- [ ] Keyboard shortcuts / accelerators — global and scoped bindings

## Controls

- [x] Button — label, hover/pressed/disabled states, onClick
- [x] ScrollViewer — H/V scrollbar, scroll position, mouse wheel, thumb drag
- [x] TextBox / TextInput — single-line editing, caret, selection, placeholder, password, submit
- [x] CheckBox — toggle with label, checked/unchecked, disabled state, keyboard (Space)
- [x] RadioButton — mutually exclusive groups, auto-uncheck siblings, keyboard (Space)
- [x] Toggle / Switch
- [x] Slider — horizontal, value/min/max/step, mouse drag (captureMouse), keyboard arrows, show value label
- [x] DropDown (selection menu, scroll, auto up/down direction)
- [x] ProgressBar — determinate (0–100%), indeterminate pulse, % in bar, label below, themed colours
- [x] Table — scrollable data table, clickable column headers with sort callback, row selection, keyboard nav
- [x] Tooltip — Tooltip::set(element, text), overlay-based popup on hover, auto-positioning within window bounds
- [x] ContextMenu — right-click popup menu with items, separators, disabled items, hover highlight, overlay/captureMouse
- [x] TabControl — scrollable tabs with < > arrow buttons when overflow, keyboard nav, hover highlight
- [ ] ListView / ItemsControl
- [ ] TreeView
- [x] Dialog / Modal — modal overlay with backdrop, draggable title bar, close (X), configurable buttons (OK/Cancel/Yes/No), word-wrapping message, Escape/Enter keys
- [ ] Toast / Notification
- [ ] Splitter / Resizable panels
- [ ] Toolbar
- [ ] Menu bar
- [ ] Expander / Accordion
- [ ] Badge / Tag

## Animation

- [x] Float animation
- [x] Color animation
- [x] Easing functions — 30+ (quad, cubic, elastic, bounce, etc.)
- [x] Repeat / loop — once, forever, times(n)
- [x] Storyboard — multiple grouped animations
- [x] Keyframe animation
- [ ] Implicit transitions — auto-animate property changes
- [ ] Spring / physics-based animation
- [ ] Path animation — animate along a curve
- [ ] Layout animation — animate child add/remove/reorder

## Systemii

- [x] Property system — GUT_PROPERTY with change signals, one-way binding
- [x] Ref counting — intrusive Ref\<T\>, make\<T\>()
- [x] Signal / event system
- [x] Z-index / stacking order
- [x] Styles / theming — CSS selectors, stylesheets, Theme with defaultLight/defaultDark
- [x] Visual tree walking — findByName, findByType, recursive
- [p] Two-way data binding — only one-way currently (PUNT)
- [ ] Resource dictionary — no generic keyed resource store
- [ ] Command system — ICommand-style abstraction for actions
- [ ] Undo / redo
- [p] Serialization — save/load UI tree from markup (PUNT)
- [p] Debug overlay — layout inspector, render stats, FPS counter (PUNT)
- [p] Accessibility — screen reader roles, labels, navigation (PUNT)

## Platform / Backend

- [x] macOS Metal backend
- [x] Windows backend — D3D11 and D3D12
- [p] Linux backend — Vulkan or OpenGL (PUNT)
- [p] Web backend — WebGPU or Canvas (PUNT)
- [p] Multiple windows (PUNT)
