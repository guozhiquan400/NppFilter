# 生成 16x16 32bpp（带 alpha 通道）的漏斗 ICO 文件
# ICO 格式：ICONDIR(6) + ICONDIRENTRY(16) + BITMAPINFOHEADER(40) + 像素XOR(W*H*4) + 掩码AND((W/8)*H, 4字节对齐)
$W = 16
$H = 16

# 像素数据：每像素 4 字节 BGRA
$pixelSize = $W * $H * 4
# AND 掩码：每像素 1 bit，行按 4 字节对齐；16 像素 = 2 字节/行，对齐到 4 字节
$andRowSize = [int]([math]::Ceiling($W / 8.0))
if ($andRowSize % 4 -ne 0) { $andRowSize = $andRowSize + (4 - $andRowSize % 4) }
$andSize = $andRowSize * $H

$bmpHeaderSize = 40
$imageSize = $bmpHeaderSize + $pixelSize + $andSize

$dirSize = 6
$entrySize = 16
$fileSize = $dirSize + $entrySize + $imageSize
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

# ICONDIR
W16 0 0          # reserved
W16 2 1          # type = 1 (icon)
W16 4 1          # count = 1

# ICONDIRENTRY @ offset 6
$bytes[6]  = [byte]$W        # width (16)
$bytes[7]  = [byte]$H        # height (16)
$bytes[8]  = 0               # color count (0 = >256)
$bytes[9]  = 0               # reserved
W16 10 1                     # planes
W16 12 32                    # bits per pixel
W32 14 $imageSize            # size of bitmap data
W32 18 ($dirSize + $entrySize)  # offset to bitmap data

# BITMAPINFOHEADER @ offset 22
$hdr = 22
W32 ($hdr +  0) 40           # header size
W32 ($hdr +  4) $W           # width
W32 ($hdr +  8) ($H * 2)     # height = H*2（XOR + AND 总高度，ICO 规定）
W16 ($hdr + 12) 1            # planes
W16 ($hdr + 14) 32           # bpp
W32 ($hdr + 16) 0            # BI_RGB
W32 ($hdr + 20) 0            # image size（0 对 BI_RGB 可接受）
W32 ($hdr + 24) 0
W32 ($hdr + 28) 0
W32 ($hdr + 32) 0
W32 ($hdr + 36) 0

# 漏斗造型：T=透明, F=主色(橙黄), E=描边深色
$art = @(
    'TTTTTTTTTTTTTTTT',
    'EEEEEEEEEEEEEEEE',
    'EFFFFFFFFFFFFFFE',
    'TEFFFFFFFFFFFFET',
    'TTEFFFFFFFFFFETT',
    'TTTEFFFFFFFFETTT',
    'TTTTEFFFFFFETTTT',
    'TTTTTEFFFFETTTTT',
    'TTTTTTEFFETTTTTT',
    'TTTTTTEFFETTTTTT',
    'TTTTTTEFFETTTTTT',
    'TTTTTTEFFETTTTTT',
    'TTTTTTEFFETTTTTT',
    'TTTTTTEFFETTTTTT',
    'TTTTTTEEEETTTTTT',
    'TTTTTTTTTTTTTTTT'
)

$pixelOffset = 22 + 40   # 像素数据起始
$andOffset   = $pixelOffset + $pixelSize

# XOR/像素数据：BMP 是自底向上
for ($row = 0; $row -lt $H; $row++) {
    $line = $art[$H - 1 - $row]    # 视觉上第一行在 $art[0]，BMP 底行对应行 0
    $rowOff = $pixelOffset + $row * $W * 4
    for ($x = 0; $x -lt $W; $x++) {
        $c = $line[$x]
        switch ($c) {
            'T' { $b = 0;   $g = 0;   $r = 0;   $a = 0   }   # 完全透明
            'F' { $b = 230; $g = 160; $r = 60;  $a = 255 }   # 橙黄主色
            'E' { $b = 110; $g = 60;  $r = 20;  $a = 255 }   # 深棕描边
            default { $b = 0; $g = 0; $r = 0; $a = 0 }
        }
        $bytes[$rowOff + $x * 4]     = $b
        $bytes[$rowOff + $x * 4 + 1] = $g
        $bytes[$rowOff + $x * 4 + 2] = $r
        $bytes[$rowOff + $x * 4 + 3] = $a
    }
}

# AND 掩码：1=透明，0=不透明
for ($row = 0; $row -lt $H; $row++) {
    $line = $art[$H - 1 - $row]
    $rowOff = $andOffset + $row * $andRowSize
    for ($x = 0; $x -lt $W; $x++) {
        $isTransparent = ($line[$x] -eq 'T')
        if ($isTransparent) {
            $byteIdx = [int]($x / 8)
            $bitIdx  = 7 - ($x % 8)
            $bytes[$rowOff + $byteIdx] = [byte]($bytes[$rowOff + $byteIdx] -bor (1 -shl $bitIdx))
        }
    }
}

$outPath = Join-Path $PSScriptRoot 'filter.ico'
[System.IO.File]::WriteAllBytes($outPath, $bytes)
Write-Host ("Written: " + $outPath + " (" + $fileSize + " bytes)")
