
set fxc="c:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Utilities\bin\x86\fxc.exe"
set out_dir=shaders
set profile=ps_2_0
%fxc% /Ges "/Fh%out_dir%/ChromaKey.h" /T%profile% "ChromaKey.fx"
%fxc% /Ges "/Fh%out_dir%/Texture.h"   /T%profile% "Texture.fx"
