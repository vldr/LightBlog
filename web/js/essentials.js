function openTab(evt, name) {
	$('#settings-tab-content > div').hide();
	$('#tab-' + name).show();
}

function dryMouth() {
	$("#create-post").submit(function(e) {
		$.ajax({
			type: "POST",
			url: "/api/post",
			data: $("#create-post").serialize(),
			success: function(data)
			{
				if (data != "Done...")
					alert(data);
				else
					location.reload();
			}
		});

		e.preventDefault();
	});
	
	$("#change-post").submit(function(e) {
		$.ajax({
			type: "POST",
			url: "/change",
			data: $("#change-post").serialize(),
			success: function(data)
			{
				if (data != "Done...")
					alert(data);
				else
					location.reload();
			}
		});

		e.preventDefault();
	});	
	
	$("#edit-post").submit(function(e) {
		$.ajax({
			type: "POST",
			url: "/edit",
			data: $("#edit-post").serialize(),
			success: function(data)
			{
				if (data != "Done...")
					alert(data);
				else
					location.reload();
			}
		});

		e.preventDefault();
	});	
	
	$("#delete-post").submit(function(e) {
		$.ajax({
			type: "POST",
			url: "/delete",
			data: $("#delete-post").serialize(),
			success: function(data)
			{
				if (data != "Done...")
					alert(data);
				else
					window.location.replace("/index");
			}
		});

		e.preventDefault();
	});
}

function loadPosts() {
	var URI = document.URL.split('/');	
	var xhr;
	var timer; 

	if (window.XMLHttpRequest) 
		xhr = new XMLHttpRequest(); 
	else if (window.ActiveXObject) 
		xhr = new ActiveXObject("Msxml2.XMLHTTP");
	else 
		throw new Error("Ajax is not supported by your browser");
	
	xhr.onreadystatechange = function () {
		timer = setTimeout(function() {
			if (xhr.readyState < 4) {
				document.getElementById("content").innerHTML = "<br><br><div class=\"loader\"></div>";	
			}
		}, 300);
		
		if (xhr.readyState === 4) {
			if (xhr.status == 200 && xhr.status < 300)
			{
				clearTimeout(timer);
				xmlDoc = xhr.responseXML; 
				document.getElementById('content').innerHTML = xhr.response;
				document.getElementById('content').innerHTML += '<div class="searchBox"><input type="text" name="txt" placeholder="Search..." onchange="search(this.value)"></div>';
				dryMouth();
			} 
		}
	}
	
	if (URI[3].length == 0 || URI[3] == 'index')
		xhr.open('GET', '/api/home');
	else
		xhr.open('GET', 'api/home/' + URI[3]);
		
	xhr.send(null);
}

function loadThisPost() {
	var URI = document.URL.split('/');	
	var xhr;
	var timer; 

	if (window.XMLHttpRequest) 
		xhr = new XMLHttpRequest(); 
	else if (window.ActiveXObject) 
		xhr = new ActiveXObject("Msxml2.XMLHTTP");
	else 
		throw new Error("Ajax is not supported by your browser");
	
	xhr.onreadystatechange = function () {
		timer = setTimeout(function() {
			if (xhr.readyState < 4) {
				document.getElementById("content").innerHTML = "<br><br><div class=\"loader\"></div>";	
			}
		}, 300);
		
		if (xhr.readyState === 4) {
			if (xhr.status == 200 && xhr.status < 300)
			{
				clearTimeout(timer);
				xmlDoc = xhr.responseXML; 
				document.getElementById('content').innerHTML = xhr.response;
				
				dryMouth();
			} 
		}
	}
	xhr.open('GET', '../api/view/' + URI[4]);
	xhr.send(null);
}

function search(val) {
	var xhr;
	var timer; 

	if (window.XMLHttpRequest) 
		xhr = new XMLHttpRequest(); 
	else if (window.ActiveXObject) 
		xhr = new ActiveXObject("Msxml2.XMLHTTP");
	else 
		throw new Error("Ajax is not supported by your browser");
	
	xhr.onreadystatechange = function () {
		timer = setTimeout(function() {
			if (xhr.readyState < 4) {
				document.getElementById("content").innerHTML = "<br><br><div class=\"loader\"></div>";	
			}
		}, 300);
		
		if (xhr.readyState === 4) {
			if (xhr.status == 200 && xhr.status < 300)
			{
				clearTimeout(timer);
				xmlDoc = xhr.responseXML; 
				
				document.getElementById('content').innerHTML = '<br><br><div class="searchBox"><input type="text" name="txt" placeholder="Search..." value="' + val + '" onchange="search(this.value)"></div>';
				document.getElementById('content').innerHTML += xhr.response;
				window.scrollTo(0, 0);
			} 
		}
	}

	xhr.open('GET', 'api/find/' + encodeURIComponent(val));	
	xhr.send(null);
}