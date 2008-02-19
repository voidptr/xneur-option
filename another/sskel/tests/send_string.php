<?php

	$total = 1;
	$tm = time();

	for($i = 0; $i < $total; $i++)
	{
		$fid = fsockopen("127.0.0.1", 3048);
		if(!$fid)
			die("Can't connect to socket");

		$str = "Test string\n";
		$len = strlen($str);

		if(isset($argv[1]))
			sleep($argv[1]);

		$writted = fwrite($fid, $str, $len);
		if($writted != $len)
			echo "Error\n";

		if(isset($argv[1]))
			sleep($argv[1]);

//		$writted = fwrite($fid, $str, $len);

		fclose($fid);
	}

	echo (time() - $tm)."\n";
?>
