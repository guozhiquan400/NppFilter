$path = 'E:\npp-filter\src\DockingFeature\filter.ico'
$b = [IO.File]::ReadAllBytes($path)
Write-Host "File size: $($b.Length) bytes"
Write-Host "Reserved (should be 0): $($b[0] + $b[1]*256)"
Write-Host "Type (should be 1 for icon): $($b[2] + $b[3]*256)"
Write-Host "Count: $($b[4] + $b[5]*256)"
Write-Host "Entry: W=$($b[6]) H=$($b[7]) Palette=$($b[8]) Planes=$($b[10]+$b[11]*256) BPP=$($b[12]+$b[13]*256)"
$dataSize = $b[14] + $b[15]*256 + $b[16]*65536 + $b[17]*16777216
$dataOffset = $b[18] + $b[19]*256 + $b[20]*65536 + $b[21]*16777216
Write-Host "Data size: $dataSize   Data offset: $dataOffset"
