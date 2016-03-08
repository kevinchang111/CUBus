var MYKEY= "";
var version = 'v2.2';
var stopName = [];
var stopID = [];
var currStop = 0;
function fetchTimes(stopNum){
	var req = new XMLHttpRequest();
	req.open('GET', 'https://developer.cumtd.com/api/'+version+'/json/GetDeparturesByStop?key='+MYKEY+'&stop_id='+stopID[stopNum]+'&pt=60&count=10', true);
	req.onload= function(){
		if(req.readyState == 4){
			if(req.status == 200){
				var response = JSON.parse(req.responseText);
				console.log('len: '+response.departures.length);
				
				var tex = '{ "1":'+(response.departures.length)+', ';
				tex = tex+' "2":"'+stopName[stopNum]+'", ';
				for( i = 1; i <= response.departures.length; i+=1){
					var headsign = response.departures[i-1].headsign;
					var ETA = response.departures[i-1].expected_mins;
					var tex = tex+'"'+(i*2+1)+'":"'+headsign+'","'+(i*2+2)+'":'+ETA;
				console.log('headsign '+headsign);
				console.log('eta '+ETA);
					if(i!= response.departures.length){ tex=tex+',';}
				}
				tex = tex+'}';
				console.log(tex);
				var thing = JSON.parse(tex);
				Pebble.sendAppMessage(thing);
			}else{console.log('Error request');}
		}
	};
	req.send(null);

}
function getLocationStop(lat, lon){
	var req = new XMLHttpRequest();
		console.log('gotLong: '+lon);
		console.log('gotLat: '+lat);
	req.open('GET', 'https://developer.cumtd.com/api/'+version+'/json/GetStopsByLatLon?key='+MYKEY+'&lat='+lat+'&lon='+lon+'&count=10', true);
	req.onload= function(){
		if(req.readyState == 4){
			if(req.status == 200){
				var response = JSON.parse(req.responseText);
				console.log('num stops: '+response.stops.length);
				
				var tex = '{ "1":'+(response.stops.length)+', ';
				for( i = 1; i <= response.stops.length; i+=1){
					console.log('stopid: '+response.stops[i-1].stop_id);
					stopName[i-1] = response.stops[i-1].stop_name;			
					console.log('stopname: '+stopName[i-1]);
					stopID[i-1] = response.stops[i-1].stop_id;				
					var tex = tex+'"'+(i+1)+'":"'+stopName[i-1]+'"';
					if(i!= response.stops.length){ tex=tex+',';}
				}
				tex = tex+'}';
				console.log(tex);
				var thing = JSON.parse(tex);
				Pebble.sendAppMessage(thing);
			}
				
		}else{console.log('Error request');}
	};
	req.send(null);
}

function getLocation(){
	console.log("get location");
	navigator.geolocation.getCurrentPosition(
			function(position){
				loc = position.coords;
				console.log("Location:");	
				console.log('Lat: ' +loc.latitude);
				console.log('Long: '+loc.longitude);
				console.log('accu: '+loc.accuracy); 
				getLocationStop(loc.latitude, loc.longitude);
			}, function(err){

				console.log('ERROR location'+err.code+' : '+err.message);
			}, {enableHighAccuracy : true, timeout: 2000, maximumAge:1000}
		);
	console.log("finish get location");
}
Pebble.addEventListener('ready', function(e){
	console.log('Javascript running');
//	fetchTimes(1);
	getLocation();
});

Pebble.addEventListener('appmessage', function(e){
	console.log('got appmsg');
	var msg = e.payload;//JSON.stringify(e.payload);
	console.log('Got :'+msg[1]);
	var type = msg[1];
	if(type == "refreshTimes"){
		fetchTimes(currStop);
	}
	else if(type == "changeStop"){
		console.log('Got :'+msg[2]);
		currStop = msg[2];	
		fetchTimes(currStop);
	}
});
