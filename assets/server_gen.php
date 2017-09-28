<?php

$autoload = realpath(__DIR__ . '/vendor/autoload.php');
if (!file_exists($autoload)) {
    echo 'You must set up the project dependencies, run the following commands:' . PHP_EOL .
        'curl -sS https://getcomposer.org/installer | php' . PHP_EOL .
        'php composer.phar install' . PHP_EOL;
    exit(1);
}
require($autoload);

use MatthiasMullie\Minify;

echo "///\n// server_data.h\n//\n\n";
$date = gmdate('D, j M Y H:i:s');
echo "static const char LAST_MODIFIED[] = \"{$date} GMT\";\n\n";

$file = 'style.css';
$minifier = new Minify\CSS($file);
$data = $minifier->gzip();
$size =  strlen($data);
$etag = substr(md5($data), 0, 6);
echo "// {$file}\n";
echo "#define STYLE_CSS_GZ_LEN {$size}\n";
echo "#define STYLE_CSS_GZ_ETAG \"{$etag}\"\n";
echo "static const uint8_t STYLE_CSS_GZ_DATA[] PROGMEM = {";
for ($i = 0; $i < strlen($data); $i++) {
    if ($i % 16 == 0) {
        echo "\n";
    }
    echo ' 0x' . str_pad(strtoupper(dechex(ord($data[$i]))), 2, '0', STR_PAD_LEFT) . ',';
}
echo "\n};\n\n";

$file = 'turn.js';
$minifier = new Minify\JS($file);
$data = $minifier->gzip();
$size =  strlen($data);
$etag = substr(md5($data), 0, 6);
echo "// {$file}\n";
echo "#define TURN_JS_GZ_LEN {$size}\n";
echo "#define TURN_JS_GZ_ETAG \"{$etag}\"\n";
echo "static const uint8_t TURN_JS_GZ_DATA[] PROGMEM = {";
for ($i = 0; $i < strlen($data); $i++) {
    if ($i % 16 == 0) {
        echo "\n";
    }
    echo ' 0x' . str_pad(strtoupper(dechex(ord($data[$i]))), 2, '0', STR_PAD_LEFT) . ',';
}
echo "\n};\n";
