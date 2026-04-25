# 生成 16x16 24bpp 漏斗图标 bitmap_funnel.bmp
# 透明色使用品红 RGB(255,0,255)，与工具栏 ImageList_AddMasked 一致
$W = 16
$H = 16
$rowSize = $W * 3
$pixelSize = $rowSize * $H
$fileSize = 54 + $pixelSize
$bytes = New-Object byte[] $fileSize

function W16([int]$o, [int]$v) {
    $bytes[$o]     = [byte]($v -band 0xFF)
    $bytes[$o + 1] = [byte](($v -shr 8) -band 0xFF)
}
function W32([int]$o, [int]$v) {
    $bytes[$o]     = [byte]($v -band 0xFF)
    $bytes[$o + 1] = [byte](($v -shr 8) -band 0xFF)
    $bytes[$o + 2] = [byte](($v -shr 16) -band 0xFF)
    $bytes[$o + 3] = [byte](($v -shr 24) -band 0xFF)
}

# BMP 文件头 14 字节
$bytes[0] = 0x42  # 'B'
$bytes[1] = 0x4D  # 'M'
W32 2  $fileSize
W32 6  0
W32 10 54         # pixel data offset

# DIB 头 BITMAPINFOHEADER 40 字节
W32 14 40
W32 18 $W
W32 22 $H
W16 26 1          # planes
W16 28 24         # bits per pixel
W32 30 0          # BI_RGB
W32 34 $pixelSize
W32 38 2835       # x ppm (72 dpi)
W32 42 2835       # y ppm
W32 46 0
W32 50 0

# 漏斗造型：M=品红透明, F=漏斗主色(橙黄), E=描边深色
# 上部倒梯形 + 中部细颈 + 底部封口
$art = @(
    'MMMMMMMMMMMMMMMM',
    'EEEEEEEEEEEEEEEE',
    'EFFFFFFFFFFFFFFE',
    'MEFFFFFFFFFFFFEM',
    'MMEFFFFFFFFFFEMM',
    'MMMEFFFFFFFFEMMM',
    'MMMMEFFFFFFEMMMM',
    'MMMMMEFFFFEMMMMM',
    'MMMMMMEFFEMMMMMM',
    'MMMMMMEFFEMMMMMM',
    'MMMMMMEFFEMMMMMM',
    'MMMMMMEFFEMMMMMM',
    'MMMMMMEFFEMMMMMM',
    'MMMMMMEFFEMMMMMM',
    'MMMMMMEEEEMMMMMM',
    'MMMMMMMMMMMMMMMM'
)

for ($row = 0; $row -lt $H; $row++) {
    # BMP 像素是自底向上存储，$art[0] 是视觉上的顶行
    $line = $art[$H - 1 - $row]
    $offset = 54 + ($H - 1 - $row) * $rowSize
    for ($x = 0; $x -lt $W; $x++) {
        $c = $line[$x]
        switch ($c) {
            'M' { $b = 255; $g = 0;   $r = 255 }
            'F' { $b = 230; $g = 160; $r = 60  }  # 橙黄主色 (BGR 存储)
            'E' { $b = 110; $g = 60;  $r = 20  }  # 深棕描边
            default { $b = 255; $g = 0; $r = 255 }
        }
        $bytes[$offset + $x * 3]     = $b
        $bytes[$offset + $x * 3 + 1] = $g
        $bytes[$offset + $x * 3 + 2] = $r
    }
}

$outPath = Join-Path $PSScriptRoot 'bitmap_funnel.bmp'
[System.IO.File]::WriteAllBytes($outPath, $bytes)
Write-Host ("Written: " + $outPath + " (" + $fileSize + " bytes)")
