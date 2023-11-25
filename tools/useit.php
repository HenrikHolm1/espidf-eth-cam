<?php
// Handling data in JSON format on the server-side using PHP
//
header("Content-Type: application/json");
// build a PHP variable from JSON sent using POST method
$j = stripslashes(file_get_contents("php://input"));
$v = json_decode($j);
// build a PHP variable from JSON sent using GET method
// $v = json_decode(stripslashes($_GET["data"]));
// encode the PHP variable to JSON and send it back on client-side
// echo json_encode($v);

//$o = "Spot imW: $v->imW, imH: $v->imH \r\n";
//$n = 

$json = preg_replace('/^[^,]*,\s*/', '{', $j);

$f = fopen($v->profileName."-useit.json", "w+");
if (!$f) die("Unable to open file!");

fwrite($f, $json);
fclose($f);

echo "SUCCESS $json";
?>

