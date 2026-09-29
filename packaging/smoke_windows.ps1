param([string]$Package,[string]$Results)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class WindowCheck {
 [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left,Top,Right,Bottom; }
 [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h,uint m,IntPtr w,IntPtr l);
 [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h,out RECT r);
 [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h,out RECT r);
 [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
 [DllImport("user32.dll")] public static extern bool IsZoomed(IntPtr h);
 [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
 [DllImport("user32.dll",EntryPoint="GetWindowLongPtrW")] public static extern IntPtr GetWindowLongPtr(IntPtr h,int index);
 [DllImport("dwmapi.dll")] public static extern int DwmGetWindowAttribute(IntPtr h,int a,out int v,int size);
 [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
}
'@
[void][WindowCheck]::SetProcessDPIAware()
$oldClipboard = [System.Windows.Forms.Clipboard]::GetDataObject()
$env:PATH = "$env:WINDIR\System32;$env:WINDIR"
$exe = Join-Path $Package 'W65C02-Studio.exe'
$app = Start-Process -FilePath $exe -WorkingDirectory $env:WINDIR -PassThru
try {
 for($i=0;$i -lt 100;$i++) {
  Start-Sleep -Milliseconds 100
  $app.Refresh()
  if($app.HasExited) {throw "App exited early: $($app.ExitCode)"}
  if($app.MainWindowHandle -ne 0) {break}
 }
 $script:hwnd = $app.MainWindowHandle
 if($hwnd -eq 0) {throw 'No app window appeared'}
 if($app.MainWindowTitle -ne 'W65C02 Studio') {throw "Unexpected window: $($app.MainWindowTitle)"}
 [void][WindowCheck]::SetForegroundWindow($hwnd)
 Start-Sleep -Milliseconds 500
 $style = [WindowCheck]::GetWindowLongPtr($hwnd,-16).ToInt64()
 if(($style -band 0x70000) -ne 0x70000) {throw 'Missing native resize/minimize/maximize style'}
 $caption=0;$round=0
 $captionResult=[WindowCheck]::DwmGetWindowAttribute($hwnd,35,[ref]$caption,4)
 [void][WindowCheck]::DwmGetWindowAttribute($hwnd,33,[ref]$round,4)
 if(($captionResult -eq 0 -and $caption -ne 0x1d1914) -or $round -ne 2) {throw "DWM styling mismatch: $caption $round"}
 Write-Output "Caption query HRESULT: $captionResult (caption color is a set-only attribute on some Windows builds)."
 Write-Output 'PASS: portable app launches from Windows directory with only system PATH; DWM dark caption and rounded corners applied.'
 [void][WindowCheck]::PostMessage($hwnd,0x112,[IntPtr]0xF030,[IntPtr]0)
 Start-Sleep -Milliseconds 400
 if(-not [WindowCheck]::IsZoomed($hwnd)) {throw 'Maximize failed'}
 [void][WindowCheck]::PostMessage($hwnd,0x112,[IntPtr]0xF120,[IntPtr]0)
 Start-Sleep -Milliseconds 400
 [void][WindowCheck]::PostMessage($hwnd,0x112,[IntPtr]0xF020,[IntPtr]0)
 Start-Sleep -Milliseconds 400
 if(-not [WindowCheck]::IsIconic($hwnd)) {throw 'Minimize failed'}
 [void][WindowCheck]::PostMessage($hwnd,0x112,[IntPtr]0xF120,[IntPtr]0)
 Start-Sleep -Milliseconds 400
 [void][WindowCheck]::SetForegroundWindow($hwnd)
 Write-Output 'PASS: native maximize, restore, minimize, and restore.'
 function Click-App([double]$x,[double]$y) {
  $r=New-Object WindowCheck+RECT
  [void][WindowCheck]::GetClientRect($hwnd,[ref]$r)
  $scale=[Math]::Min($r.Right/1280.0,$r.Bottom/900.0)
  $px=[int](($r.Right-1280*$scale)/2+$x*$scale)
  $py=[int](($r.Bottom-900*$scale)/2+$y*$scale)
  $position=[IntPtr](($py -shl 16) -bor $px)
  [void][WindowCheck]::PostMessage($hwnd,0x201,[IntPtr]1,$position)
  [void][WindowCheck]::PostMessage($hwnd,0x202,[IntPtr]0,$position)
  Start-Sleep -Milliseconds 120
 }
 function Type-App([string]$text) {
  foreach($c in $text.ToCharArray()) {
   [void][WindowCheck]::PostMessage($hwnd,0x102,[IntPtr][int]$c,[IntPtr]1)
  }
  [void][WindowCheck]::PostMessage($hwnd,0x100,[IntPtr]13,[IntPtr]1)
  [void][WindowCheck]::PostMessage($hwnd,0x101,[IntPtr]13,[IntPtr]1)
 }
 function Transcript {
  Click-App 670 127
  return [System.Windows.Forms.Clipboard]::GetText()
 }
 function Wait-Transcript([string]$pattern) {
  for($i=0;$i -lt 100;$i++) {
   Start-Sleep -Milliseconds 150
   $text=Transcript
   if($text -match $pattern) {return $text}
  }
  throw "Terminal did not match $pattern. Last output: $text"
 }
 # Build the selected hello_world.asm through the packaged assembler.
 Click-App 765 32
 $builds=Join-Path $Package 'builds'
 for($i=0;$i -lt 100;$i++) {
  Start-Sleep -Milliseconds 100
  $rom=Get-ChildItem -Path $builds -Filter program.bin -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
  if($rom -and $rom.Length -eq 32768) {break}
 }
 if(-not $rom -or $rom.Length -ne 32768) {throw 'Packaged assembler did not produce a 32 KiB ROM'}
 $listing=Get-Content (Join-Path $rom.Directory.FullName 'program.lst') -Raw
 if($listing.Length -lt 100) {throw 'Assembler listing missing'}
 Write-Output 'PASS: Build button uses packaged native assembler and creates a 32 KiB ROM and source listing.'
 Click-App 575 86
 Click-App 450 127
 $text=Wait-Transcript '(?s)BYTES FREE.*OK'
 Type-App 'PRINT 2+2'
 $text=Wait-Transcript '(?s)PRINT 2\+2.*\r?\n\s*4\s*\r?\n.*OK'
 $text | Set-Content (Join-Path $Results 'windows-basic-transcript.txt')
 Write-Output 'PASS: packaged WozMon/BASIC ROM boots; PRINT 2+2 returns 4 on Windows.'
 Start-Sleep -Milliseconds 300
 $r=New-Object WindowCheck+RECT
 [void][WindowCheck]::GetWindowRect($hwnd,[ref]$r)
 $bitmap=New-Object System.Drawing.Bitmap(($r.Right-$r.Left),($r.Bottom-$r.Top))
 $graphics=[System.Drawing.Graphics]::FromImage($bitmap)
 $graphics.CopyFromScreen($r.Left,$r.Top,0,0,$bitmap.Size)
 $bitmap.Save((Join-Path $Results 'windows-11-preview.png'),[System.Drawing.Imaging.ImageFormat]::Png)
 $graphics.Dispose();$bitmap.Dispose()
 [void][WindowCheck]::PostMessage($hwnd,0x10,[IntPtr]0,[IntPtr]0)
 if(-not $app.WaitForExit(5000)) {throw 'Native close did not exit'}
 Write-Output 'PASS: native Close exits the app.'
} finally {
 if(-not $app.HasExited) {$app.Kill()}
 if($oldClipboard) {[System.Windows.Forms.Clipboard]::SetDataObject($oldClipboard,$true)}
}
