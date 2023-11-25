<?php
// PHP program to delete a file named gfg.txt 
// using unlink() function 

error_reporting(E_ERROR | E_WARNING | E_PARSE);

ini_set('display_errors', '1');

$j = stripslashes(file_get_contents("php://input"));
$v = json_decode($j);


if(!file_exists("json/$v->oldFileName")){
    echo ("ERROR: File with that name is not found. Got: $v->oldFileName");
    return;
}

if(file_exists("json/$v->newFileName")){
    echo ("ERROR: A file with that name already exists. Got: $v->newFileName");
    return;
}

if (!rename("json/$v->oldFileName", "json/$v->newFileName")){
    echo ("ERROR: Could not rename. JSON received: $j");
}
else { 
    echo ("SUCCESS: $v->oldFileName has been renamed to $v->newFileName"); 
} 
 
?> 

