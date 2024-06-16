$Dimensions = @("16","20","24","32","40","48","60","64","72","80","96","256")

$Files = ""
mkdir "Generated"
foreach ($dimension in $Dimensions)
{
	$file = "Generated/Icon-$dimension.png"
	magick Base.png -resize $dimensionx$dimension $file
	$Files += "$file "
}

magick $Files "../../Resource/icon.ico"