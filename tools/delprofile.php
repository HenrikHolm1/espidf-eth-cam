<?php
// PHP program to delete a file named gfg.txt 
// using unlink() function 
  
//$file_pointer = "filetobedeleted.txt"; 

$j = stripslashes(file_get_contents("php://input"));
$v = json_decode($j);

$file_pointer = "json/$v->fileName";

// Use unlink() function to delete a file 
if (!unlink($file_pointer)) { 
    echo ("ERROR: $file_pointer cannot be deleted due to an error $j"); 
} 
else { 
    echo ("SUCCESS: $file_pointer has been deleted"); 
} 
 
?> 

