var Clay = require('pebble-clay');

var clayConfig = [
  { "type": "heading", "defaultValue": "Wyrd Loom Settings" },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Dynamic Theme Mode" },
      {
        "type": "select",
        "messageKey": "ThemeMode",
        "defaultValue": "0",
        "label": "Theme Switching",
        "options": [
          { "label": "Single Theme (Always Day Colors)", "value": "0" },
          { "label": "Manual Times", "value": "1" },
          { "label": "Auto Sunrise/Sunset (Uses Phone GPS)", "value": "2" }
        ]
      },
      { "type": "input", "messageKey": "ManualDayTime", "defaultValue": "07:00", "label": "Manual Day Start (HH:MM)", "attributes": { "type": "time" } },
      { "type": "input", "messageKey": "ManualNightTime", "defaultValue": "19:00", "label": "Manual Night Start (HH:MM)", "attributes": { "type": "time" } }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Day Colors" },
      { "type": "color", "messageKey": "BackgroundColor", "defaultValue": "0x000000", "label": "Background" },
      { "type": "color", "messageKey": "MinuteHandColor", "defaultValue": "0xFF5500", "label": "Minute Hand" },
      { "type": "color", "messageKey": "HourColor", "defaultValue": "0xFFFFFF", "label": "Hour Numbers" },
      { "type": "color", "messageKey": "MinuteMarkerColor", "defaultValue": "0xAAAAAA", "label": "Minute Markers" },
      { "type": "color", "messageKey": "CompCircleColor", "defaultValue": "0xFF5500", "label": "Pill Outline" },
      { "type": "color", "messageKey": "CompFillColor", "defaultValue": "0xFF5500", "label": "Pill Fill" },
      { "type": "color", "messageKey": "CompTextColor", "defaultValue": "0xFFFFFF", "label": "Pill Text" }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "id": "heading_night_colors", "defaultValue": "Night Colors" },
      { "type": "color", "messageKey": "NightBackgroundColor", "defaultValue": "0x000000", "label": "Background" },
      { "type": "color", "messageKey": "NightMinuteHandColor", "defaultValue": "0x0055AA", "label": "Minute Hand" },
      { "type": "color", "messageKey": "NightHourColor", "defaultValue": "0xAAAAAA", "label": "Hour Numbers" },
      { "type": "color", "messageKey": "NightMinuteMarkerColor", "defaultValue": "0x555555", "label": "Minute Markers" },
      { "type": "color", "messageKey": "NightCompCircleColor", "defaultValue": "0x0055AA", "label": "Pill Outline" },
      { "type": "color", "messageKey": "NightCompFillColor", "defaultValue": "0x0055AA", "label": "Pill Fill" },
      { "type": "color", "messageKey": "NightCompTextColor", "defaultValue": "0xAAAAAA", "label": "Pill Text" }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Dial Styling" },
      { "type": "select", "messageKey": "HourStyle", "defaultValue": "0", "label": "Hour Style", "options": [ { "label": "Standard Numbers", "value": "0" }, { "label": "Roman Numerals", "value": "1" } ] },
      { "type": "select", "messageKey": "HourPosition", "defaultValue": "0", "label": "Hour Position", "options": [ { "label": "On Top of Track", "value": "0" }, { "label": "Outside Track", "value": "1" }, { "label": "Inside Track", "value": "2" } ] },
      { "type": "slider", "messageKey": "MinuteMarkerInterval", "defaultValue": 15, "label": "Marker Interval", "min": 5, "max": 30, "step": 1 },
      { "type": "select", "messageKey": "MinuteMarkerStyle", "defaultValue": "2", "label": "Marker Style", "options": [ { "label": "Small Dot", "value": "0" }, { "label": "Big Dot", "value": "1" }, { "label": "Short Line", "value": "2" }, { "label": "Long Line", "value": "3" } ] },
      { "type": "toggle", "messageKey": "SmartHierarchicalMarkers", "label": "Smart Highlight Markers", "defaultValue": true }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Complications" },
      { "type": "select", "messageKey": "Complication1", "defaultValue": "3", "label": "Top", "options": [ { "label": "None", "value": "0" }, { "label": "Steps", "value": "1" }, { "label": "Day and Month", "value": "2" }, { "label": "Day", "value": "3" }, { "label": "Battery", "value": "5" }, { "label": "Heart Rate", "value": "6" }, { "label": "Day of Week", "value": "7" }, { "label": "AM/PM", "value": "8" }, { "label": "Week Number", "value": "9" }, { "label": "Year", "value": "10" }, { "label": "Distance Walked", "value": "11" }, { "label": "Calories Burned", "value": "12" }, { "label": "Active Time", "value": "13" }, { "label": "Compass Heading", "value": "14" } ] },
      { "type": "select", "messageKey": "Complication2", "defaultValue": "0", "label": "Middle", "options": [ { "label": "None", "value": "0" }, { "label": "Steps", "value": "1" }, { "label": "Day and Month", "value": "2" }, { "label": "Day", "value": "3" }, { "label": "Battery", "value": "5" }, { "label": "Heart Rate", "value": "6" }, { "label": "Day of Week", "value": "7" }, { "label": "AM/PM", "value": "8" }, { "label": "Week Number", "value": "9" }, { "label": "Year", "value": "10" }, { "label": "Distance Walked", "value": "11" }, { "label": "Calories Burned", "value": "12" }, { "label": "Active Time", "value": "13" }, { "label": "Compass Heading", "value": "14" } ] },
      { "type": "select", "messageKey": "Complication3", "defaultValue": "0", "label": "Bottom", "options": [ { "label": "None", "value": "0" }, { "label": "Steps", "value": "1" }, { "label": "Day and Month", "value": "2" }, { "label": "Day", "value": "3" }, { "label": "Battery", "value": "5" }, { "label": "Heart Rate", "value": "6" }, { "label": "Day of Week", "value": "7" }, { "label": "AM/PM", "value": "8" }, { "label": "Week Number", "value": "9" }, { "label": "Year", "value": "10" }, { "label": "Distance Walked", "value": "11" }, { "label": "Calories Burned", "value": "12" }, { "label": "Active Time", "value": "13" }, { "label": "Compass Heading", "value": "14" } ] }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Behavior" },
      { "type": "select", "messageKey": "BTDisconnectPattern", "defaultValue": "2", "label": "BT Disconnect Alert", "options": [ { "label": "None", "value": "0" }, { "label": "Short Pulse", "value": "1" }, { "label": "Double Pulse", "value": "2" }, { "label": "Long Pulse", "value": "3" } ] },
      { "type": "toggle", "messageKey": "ShowMinuteBubble", "label": "Show minute bubble", "defaultValue": false },
      { "type": "toggle", "messageKey": "HandOverNumbers", "label": "Hand over numbers", "defaultValue": true },
      { "type": "toggle", "messageKey": "ShowMinuteMarkers", "label": "Show minute markers", "defaultValue": true },
      { "type": "toggle", "messageKey": "HandLengthScreenEdge", "label": "Hand goes to edge", "defaultValue": true }
    ]
  },
  { "type": "submit", "defaultValue": "Save Settings" }
];

function customClayJS() {
  var clayConfig = this;

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    
    // --- SMART BUBBLE LOGIC ---
    var toggleBubble = clayConfig.getItemByMessageKey('ShowMinuteBubble');
    var posHour = clayConfig.getItemByMessageKey('HourPosition');
    if (toggleBubble && posHour) {
      toggleBubble.on('change', function() { if (toggleBubble.get()) posHour.set('1'); });
    }

    // --- SMART THEME UI & LIVE API LOGIC ---
    var themeMode = clayConfig.getItemByMessageKey('ThemeMode');
    var manualDay = clayConfig.getItemByMessageKey('ManualDayTime');
    var manualNight = clayConfig.getItemByMessageKey('ManualNightTime');
    
    var nightHeading = clayConfig.getItemById('heading_night_colors');
    var nightKeys = [
      'NightBackgroundColor', 'NightMinuteHandColor', 'NightHourColor', 
      'NightMinuteMarkerColor', 'NightCompCircleColor', 'NightCompFillColor', 'NightCompTextColor'
    ];

    function fetchLiveSunData() {
      if(navigator.geolocation) {
        navigator.geolocation.getCurrentPosition(
          function(pos) {
            var lat = pos.coords.latitude;
            var lon = pos.coords.longitude;
            var url = 'https://api.sunrise-sunset.org/json?lat=' + lat + '&lng=' + lon + '&formatted=0';
            
            var req = new XMLHttpRequest();
            req.open('GET', url, true);
            req.onload = function() {
              if (req.readyState === 4 && req.status === 200) {
                var res = JSON.parse(req.responseText);
                var sunrise = new Date(res.results.sunrise);
                var sunset = new Date(res.results.sunset);
                
                // Format directly into standard HH:MM for the time inputs
                var sr_h = sunrise.getHours().toString().padStart(2, '0');
                var sr_m = sunrise.getMinutes().toString().padStart(2, '0');
                var ss_h = sunset.getHours().toString().padStart(2, '0');
                var ss_m = sunset.getMinutes().toString().padStart(2, '0');
                
                // Magically inject the calculated times directly into the locked UI text boxes!
                manualDay.set(sr_h + ':' + sr_m);
                manualNight.set(ss_h + ':' + ss_m);
              }
            };
            req.send(null);
          },
          function(err) { console.log("User denied or GPS failed."); },
          { timeout: 15000, maximumAge: 60000 }
        );
      }
    }

    function updateThemeUI() {
      var mode = themeMode.get();

      // Handle Manual Time Selectors
      if (mode === "1") {
        manualDay.enable();
        manualNight.enable();
      } else {
        manualDay.disable();
        manualNight.disable();
      }

      // Automatically trigger the API fetch if Auto mode is selected
      if (mode === "2") {
        fetchLiveSunData();
      }

      // Handle Night Colors Visibility
      if (mode === "0") {
        if (nightHeading) nightHeading.hide();
        nightKeys.forEach(function(k) { 
          var item = clayConfig.getItemByMessageKey(k);
          if(item) item.hide(); 
        });
      } else {
        if (nightHeading) nightHeading.show();
        nightKeys.forEach(function(k) { 
          var item = clayConfig.getItemByMessageKey(k);
          if(item) item.show(); 
        });
      }
    }

    updateThemeUI();
    themeMode.on('change', updateThemeUI);
  });
}

var clay = new Clay(clayConfig, customClayJS, { autoHandleEvents: false });

Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL(clay.generateUrl());
});

// Since the UI now pre-fills the data dynamically, we just send whatever is inside Clay
Pebble.addEventListener('webviewclosed', function(e) {
  if (e && !e.response) return;
  var dict = clay.getSettings(e.response);
  Pebble.sendAppMessage(dict);
});