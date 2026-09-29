# CPU application icon

Generated with the built-in image_gen tool. The original artwork is `cpu-source.png`; `cpu.png` is the 256-pixel app/launcher icon, and `cpu.ico` includes 16, 24, 32, 48, 64, 128, and 256-pixel Windows icons. The PNG alpha channel is preserved during resizing and format conversion.

## Generation prompt

```text
Use case: logo-brand
Asset type: Windows and Linux desktop application icon for a 65C02 CPU emulator.
Primary request: A polished, instantly recognizable CPU microchip icon.
Subject: A single square dark graphite CPU package viewed straight from above, with bold metallic pins on all four sides and a simple inset silicon die. Subtle dimensional bevels and crisp highlights give it a premium hardware feel.
Composition: Centered, square canvas, large chip silhouette with generous safe margin so all pins remain inside the frame. Strong simple geometry and high contrast, readable when reduced to 32 or 16 pixels.
Backdrop: genuinely transparent background, preserve alpha; no backdrop tile, no surface, no surrounding objects.
Constraints: no text, no letters, no logos, no watermark, no tiny circuit detail, no dramatic perspective.
```

CMake embeds the PNG in the application window code and the ICO in the Windows executable. Rebuild `main` after changing either asset. Linux builds also generate `SystemLib/w65c02-studio.desktop` in the build directory; copy that file to `~/.local/share/applications/` to add the app to your desktop menu. That launcher refers to this build directory, so regenerate and replace it if the build is moved.
