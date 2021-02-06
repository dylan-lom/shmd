#!/usr/bin/env sh
md="$(cat)"

if (echo "$md" | grep -q '</head>'); then
    md="$(echo "$md" | sed 's/<\/head>/<\/head>\n<body>/')"
else
    md="$(echo "<body>\n$md")"
fi

echo "<!DOCTYPE html>\n<html>\n$md\n</body>\n</html>"
