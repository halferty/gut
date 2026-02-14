# Gut Toolkit Roadmap

**Score: 46 done, 10 partial, 9 missing** out of 63 core features.

---

## Layout

- [x] Canvas — absolute positioning with attached Left/Top
- [x] StackPanel — vertical/horizontal auto-stacking
- [x] Grid — rows/columns with Auto/Pixel/Star sizing, spans
- [x] Margin / Padding — on all elements
- [x] Alignment — HorizontalAlignment, VerticalAlignment
- [x] Min/Max constraints — minWidth, maxWidth, minHeight, maxHeight
- [ ] WrapPanel — flow layout with line wrapping
- [ ] DockPanel — dock children to edges
- [ ] ViewBox — scale-to-fit container
- [ ] Anchor layout — left+right pins = auto-stretch on resize

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
- [ ] Radial gradients — brush class exists, rendering not wired
- [ ] Inset / inner shadows
- [ ] Path rendering — Path class fully designed, fill/stroke stubbed (draw bounding rect only)
- [ ] Polylines — only single line segments, no multi-segment API
- [ ] Transforms — rotation, scale, skew on elements
- [x] Backdrop blur / frosted glass
- [ ] Blend modes — multiply, screen, overlay, etc.
- [ ] 9-slice / 9-patch image rendering
- [ ] Render-to-texture — offscreen render targets

## Text

- [x] Text element — stb_truetype glyph atlas rendering
- [x] Font size
- [ ] Font weight / style — properties declared, not used for font selection
- [x] Text alignment — Left, Center, Right applied during render via drawTextMultiline
- [x] Text wrapping — word wrap via TextShaper::shapeMultiline in render path
- [x] Text truncation / ellipsis — CharacterEllipsis + WordEllipsis, single-line
- [ ] Multiple font families — fontFamily property exists, no font matching
- [ ] Text selection
- [ ] Rich text / inline formatting — bold, italic, color spans within one block
- [ ] Text shadow
- [ ] Text outline / stroke

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
- [ ] Cursor styles — enum declared, not connected to OS
- [~] Clipboard — TextBox uses NSPasteboard; no general API yet
- [ ] IME input — CJK composition support
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
- [ ] Tooltip
- [ ] ContextMenu
- [x] TabControl
- [ ] ListView / ItemsControl
- [ ] TreeView
- [ ] Dialog / Modal
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

## System

- [x] Property system — GUT_PROPERTY with change signals, one-way binding
- [x] Ref counting — intrusive Ref\<T\>, make\<T\>()
- [x] Signal / event system
- [x] Z-index / stacking order
- [x] Styles / theming — CSS selectors, stylesheets, Theme with defaultLight/defaultDark
- [x] Visual tree walking — findByName, findByType, recursive
- [ ] Two-way data binding — only one-way currently
- [ ] Resource dictionary — no generic keyed resource store
- [ ] Command system — ICommand-style abstraction for actions
- [ ] Undo / redo
- [ ] Serialization — save/load UI tree from markup
- [ ] Debug overlay — layout inspector, render stats, FPS counter
- [ ] Accessibility — screen reader roles, labels, navigation

## Platform / Backend

- [x] macOS Metal backend
- [ ] Windows backend — D3D11 or D3D12
- [ ] Linux backend — Vulkan or OpenGL
- [ ] Web backend — WebGPU or Canvas
- [ ] Multiple windows
