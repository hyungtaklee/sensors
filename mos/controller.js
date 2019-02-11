var mqtt = require('mqtt');
var mysql = require("mysql");

/* set mariaDB server information */
var connection = mysql.createConnection ({
	host:		'127.0.0.1',
	port:		3306,
	user:		'cclab',
	password:	'coco2006',
	database:	'sensor'
});

/* connect to Mosquitto broker */
var client = mqtt.connect("mqtt://127.0.0.1:1883");

/* connect to mariaDB server */
connection.connect();

client.on('connect', () => {
	console.log("Connected!");
});

/* subscribe sensor data and app request through mosquitto broker */
client.subscribe('sensor/#', {qos: 0})
client.subscribe('req/#', {qos: 0});

/* when received messge */
client.on('message', function (topic, message) {
	console.log(message.toString());
	
	var topic_arr = topic.split('/');
	
	/* save sensor data */
	if (topic_arr[0] === 'sensor') {
		
		/* check table list and make new table if not exists. */
		/* table structure
		 *
		 * counter | name | value | time
		 *
		 */
		var sql_table = "CREATE TABLE IF NOT EXISTS " + topic_arr[1] + " (\n"
				+ "cnt int NOT NULL AUTO_INCREMENT PRIMARY KEY,\n"
				+ "name varchar(20) default null,\n"
				+ "value char(20) default null,\n"
				+ "time TIMESTAMP NOT NULL DEFAULT now()\n"
				+ ")";

			
		connection.query(sql_table, function (err, result) {
			if (err) throw err;
		
		});

		/* insert sensor data into table */
		var sql_insert = "INSERT INTO "
				+ topic_arr[1] 
				+ " (name, value) VALUES "
				+ "("
				+ "'" + topic_arr[2] + "'"
				+ ", "
				+ "'" + message.toString() + "'"
				+ ")";

		console.log("insert query\n" + sql_insert);
	
		connection.query(sql_insert, function (err, result) {
			if (err) throw err;
			console.log("name: " + topic_arr[1]
				+ " value: " +topic_arr[2]);
		});
	}

	/* handling request from android application */
	else { /* topic_arr[0] === 'req' */
		/* 
		 * request list (if needed add more request)
		 *
		 * req/list
		 *  list all table name (kind of sensor)
		 *
		 * req/show/<kind_of_sensor>
		 *  shows recent 30 data of <kind_of_sensor>
		 *
		 * req/show/<kind_of_sensor>/<sensor_name>
		 *  shows recent 30 data of <sensor_name>
		 *
		 * req/avg/<kind_of_sensor>/<sensor_name>
		 *  shows average value of recent 10 data of <sensor_name> 
		 * 
		 * RESULT is send as JSON type!
		 *
		 */

		/* req/list */
		if (topic_arr[1] === 'list') {
			var list_query = "SHOW TABLES;";

			connection.query(list_query, function(err, result) {
				if (err) throw err;	

				console.log('printing result...')
				console.log(result);

				var device_list = new Array();

				result.forEach(function(tbl) {
					var data = new Object();
					data.name = tbl.Tables_in_sensor;

					device_list.push(data);
				});

				var list_json = JSON.stringify(device_list);

				/* send result to mqtt server */
				client.publish('resp/list', list_json, function() {
						console.log('resp/list');
						console.log(list_json);
					}
				);

			});
		}
		/* req/show/<kind_of_sensor> */
		else if (topic_arr.length === 3 && topic_arr[1] === 'show') {
			var query = "SELECT name, value, time FROM "
				+ topic_arr[2]
				+ " limit 30;";
			connection.query(query, function(err, result) {
					/* 안드로이드 앱 측에서 리스트를 받아서 존재하는 테이블만을 조회하므로 문제없음 */
					if (err) throw err;
		
					console.log('print result of /req/show/' + topic_arr[2]);
					console.log(result);

					var data_list = new Array();

					var cnt = 1;
					result.forEach(function(elem) {
						var data = new Object();
					
						data.cnt = cnt;
						data.name = elem.name;
						data.value = elem.value;
						data.time = elem.time;
						
						data_list.push(data);

						cnt = cnt + 1;
					});

					var list_json = JSON.stringify(data_list);

					/* send data to mqtt broker */
					client.publish('resp/show/' + topic_arr[2], list_json, function() {
						console.log('mqtt information of /req/show' + topic_arr[2]);
						console.log(list_json);
					});
			});
		}

		/* req/show/<kind_of_sensor>/<sensor_name> */
		else if (topic_arr.length === 4 && topic_arr[1] === 'show') {
			var query = "SELECT name, value, time FROM "
				+ topic_arr[2]
				+ " WHERE name='"
				+ topic_arr[3]
				+ "' limit 30;";

			connection.query(query, function(err, result) {
					/* 안드로이드 앱 측에서 리스트를 받아서 존재하는 테이블만을 조회하므로 문제없음 */
					if (err) throw err;
					
					console.log('print result of /req/show/' + topic_arr[2] + '/' + topic_arr[3]);
					console.log(result);

					var data_list = new Array();

					var cnt = 1;
					result.forEach(function(elem) {
						var data = new Object();
						
						data.cnt = cnt;
						data.name = elem.name;
						data.value = elem.value;
						data.time = elem.time;
						
						data_list.push(data);

						cnt = cnt + 1;
					});
					var list_json = JSON.stringify(data_list);

					/* send data to mqtt broker */
					client.publish('resp/show/' + topic_arr[2] + '/' + topic_arr[3], list_json, function() {
						console.log('mqtt information of /req/show/' + topic_arr[2] + '/' + topic_arr[3]);
						console.log(list_json);
					});
			});
		}

		else if (topic_arr[1] === 'avg') {
			var query = "SELECT AVG(value) FROM "
						+ topic_arr[2]
						+ " WHERE name='"
						+ topic_arr[3]
						+ "' LIMIT 10;"

			connection.query(query, function (err, result) {
				if (err) throw err;

				console.log('result of req/avg/');
				console.log(result);

				var data = new Array();
				
				data.push(result);

				var data_json = JSON.stringify(data);

				client.publish('resp/avg/' + topic_arr[2] + '/' + topic_arr[3], data_json, function() {
					console.log('mqtt information of /req/avg/' + topic_arr[2] + '/' + topic_arr[3]);
					console.log(data_json);
				});
			});
		}
		else {
			console.log('error: Received unknown request!');
			console.log(topic_arr);
		}
	}
});
