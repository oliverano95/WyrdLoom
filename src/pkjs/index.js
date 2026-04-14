// Import the Clay package
var Clay = require('pebble-clay');

// Construct the configuration menu
var clayConfig = [
  {
    "type": "heading",
    "defaultValue": "Wyrd Loom Settings"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors"
      },
      { "type": "color", "messageKey": "BackgroundColor", "defaultValue": "0x000000", "label": "Background Color" },
      { "type": "color", "messageKey": "MinuteHandColor", "defaultValue": "0xFF5500", "label": "Minute Hand Color" },
      { "type": "color", "messageKey": "HourColor", "defaultValue": "0xFFFFFF", "label": "Hour Numbers Color" },
      { "type": "color", "messageKey": "MinuteMarkerColor", "defaultValue": "0xAAAAAA", "label": "Minute Marker Color" },
      { "type": "color", "messageKey": "CompCircleColor", "defaultValue": "0xFF5500", "label": "Center Complication Outline" },
      { "type": "color", "messageKey": "CompFillColor", "defaultValue": "0xFF5500", "label": "Center Complication Fill" },
      { "type": "color", "messageKey": "CompTextColor", "defaultValue": "0xFFFFFF", "label": "Center Complication Text" }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Dial Styling" },
      { "type": "select", "messageKey": "HourStyle", "defaultValue": "0", "label": "Hour Style", "options": [ { "label": "Standard Numbers", "value": "0" }, { "label": "Roman Numerals", "value": "1" } ] },
      { "type": "select", "messageKey": "HourPosition", "defaultValue": "0", "label": "Hour Number Position", "options": [ { "label": "On Top of Track", "value": "0" }, { "label": "Outside Track", "value": "1" }, { "label": "Inside Track", "value": "2" } ] },
      { "type": "slider", "messageKey": "MinuteMarkerInterval", "defaultValue": 15, "label": "Marker Interval (Minutes)", "min": 5, "max": 30, "step": 1 },
      { "type": "select", "messageKey": "MinuteMarkerStyle", "defaultValue": "2", "label": "Standard Marker Style", "options": [ { "label": "Small Dot", "value": "0" }, { "label": "Big Dot", "value": "1" }, { "label": "Short Line", "value": "2" }, { "label": "Long Line", "value": "3" } ] },
      { "type": "toggle", "messageKey": "SmartHierarchicalMarkers", "label": "Smart Highlight Markers", "defaultValue": true }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Complications" },
      { "type": "select", "messageKey": "Complication1", "defaultValue": "3", "label": "Top Complication", "options": [ { "label": "None", "value": "0" }, { "label": "Steps", "value": "1" }, { "label": "Day and Month", "value": "2" }, { "label": "Day", "value": "3" }, { "label": "Battery", "value": "5" }, { "label": "Heart Rate", "value": "6" }, { "label": "Day of Week", "value": "7" }, { "label": "AM/PM", "value": "8" }, { "label": "Week Number", "value": "9" }, { "label": "Year", "value": "10" }, { "label": "Distance Walked", "value": "11" }, { "label": "Calories Burned", "value": "12" }, { "label": "Active Time", "value": "13" }, { "label": "Compass Heading", "value": "14" } ] },
      { "type": "select", "messageKey": "Complication2", "defaultValue": "0", "label": "Middle Complication", "options": [ { "label": "None", "value": "0" }, { "label": "Steps", "value": "1" }, { "label": "Day and Month", "value": "2" }, { "label": "Day", "value": "3" }, { "label": "Battery", "value": "5" }, { "label": "Heart Rate", "value": "6" }, { "label": "Day of Week", "value": "7" }, { "label": "AM/PM", "value": "8" }, { "label": "Week Number", "value": "9" }, { "label": "Year", "value": "10" }, { "label": "Distance Walked", "value": "11" }, { "label": "Calories Burned", "value": "12" }, { "label": "Active Time", "value": "13" }, { "label": "Compass Heading", "value": "14" } ] },
      { "type": "select", "messageKey": "Complication3", "defaultValue": "0", "label": "Bottom Complication", "options": [ { "label": "None", "value": "0" }, { "label": "Steps", "value": "1" }, { "label": "Day and Month", "value": "2" }, { "label": "Day", "value": "3" }, { "label": "Battery", "value": "5" }, { "label": "Heart Rate", "value": "6" }, { "label": "Day of Week", "value": "7" }, { "label": "AM/PM", "value": "8" }, { "label": "Week Number", "value": "9" }, { "label": "Year", "value": "10" }, { "label": "Distance Walked", "value": "11" }, { "label": "Calories Burned", "value": "12" }, { "label": "Active Time", "value": "13" }, { "label": "Compass Heading", "value": "14" } ] }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Behavior" },
      { "type": "select", "messageKey": "BTDisconnectPattern", "defaultValue": "2", "label": "Bluetooth Disconnect Alert", "options": [ { "label": "None", "value": "0" }, { "label": "Short Pulse", "value": "1" }, { "label": "Double Pulse", "value": "2" }, { "label": "Long Pulse", "value": "3" } ] },
      { "type": "toggle", "messageKey": "ShowMinuteBubble", "label": "Show minute bubble on hand", "defaultValue": false },
      { "type": "toggle", "messageKey": "HandOverNumbers", "label": "Draw minute hand over numbers", "defaultValue": false },
      { "type": "toggle", "messageKey": "ShowMinuteMarkers", "label": "Show minute markers", "defaultValue": true },
      { "type": "toggle", "messageKey": "HandLengthScreenEdge", "label": "Hand goes to edge of screen", "defaultValue": true }
    ]
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Tools & Sharing" },
      { "type": "button", "id": "btn_randomize", "defaultValue": "Randomize Colors" },
      { "type": "button", "id": "btn_default", "defaultValue": "Restore Default Theme" },
      { "type": "input", "id": "io_box", "label": "Import / Export Theme Code", "description": "Copy this code to save your theme, or paste a code and click Import to apply it.", "defaultValue": "" },
      { "type": "button", "id": "btn_export", "defaultValue": "Generate Theme Code" },
      { "type": "button", "id": "btn_import", "defaultValue": "Import Theme Code" }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];

// --- CUSTOM JAVASCRIPT ENGINE ---
function customClayJS() {
  var clayConfig = this;

  function snapToPebbleHex(r, g, b) {
    var pr = Math.round(r / 85) * 85; var pg = Math.round(g / 85) * 85; var pb = Math.round(b / 85) * 85;
    var hr = pr.toString(16).padStart(2, '0').toUpperCase(); var hg = pg.toString(16).padStart(2, '0').toUpperCase(); var hb = pb.toString(16).padStart(2, '0').toUpperCase();
    return '0x' + hr + hg + hb;
  }

  function getPebbleColor() {
    var hex = ['00', '55', 'AA', 'FF'];
    var r = hex[Math.floor(Math.random() * 4)]; var g = hex[Math.floor(Math.random() * 4)]; var b = hex[Math.floor(Math.random() * 4)];
    return '0x' + r + g + b;
  }

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    var btnM3Upload = clayConfig.getItemById('btn_m3_upload');
    var btnRandomize = clayConfig.getItemById('btn_randomize');
    var btnDefault = clayConfig.getItemById('btn_default');
    var btnExport = clayConfig.getItemById('btn_export');
    var btnImport = clayConfig.getItemById('btn_import');
    var ioBox = clayConfig.getItemById('io_box');
    
    // SMART UX: Auto-shift hour numbers when Bubble is activated
    var toggleBubble = clayConfig.getItemByMessageKey('ShowMinuteBubble');
    var posHour = clayConfig.getItemByMessageKey('HourPosition');
    if (toggleBubble && posHour) {
      toggleBubble.on('change', function() {
        if (toggleBubble.get()) posHour.set('1'); 
      });
    }

    var configKeys = [
      'BackgroundColor', 'MinuteHandColor', 'HourColor', 'MinuteMarkerColor', 
      'CompCircleColor', 'CompFillColor', 'CompTextColor',
      'HourStyle', 'HourPosition', 'MinuteMarkerInterval', 'MinuteMarkerStyle', 'SmartHierarchicalMarkers',
      'Complication1', 'Complication2', 'Complication3',
      'BTDisconnectPattern', 'ShowMinuteBubble', 'HandOverNumbers', 
      'ShowMinuteMarkers', 'HandLengthScreenEdge'
    ];

    if (btnRandomize) {
      btnRandomize.on('click', function() {
        var colorKeys = ['BackgroundColor', 'MinuteHandColor', 'HourColor', 'MinuteMarkerColor', 'CompCircleColor', 'CompFillColor', 'CompTextColor'];
        colorKeys.forEach(function(key) {
          var item = clayConfig.getItemByMessageKey(key);
          if (item) item.set(getPebbleColor());
        });
      });
    }

    if (btnDefault) {
      btnDefault.on('click', function() {
        var defaultSettings = {
          "BackgroundColor": "0x000000", "MinuteHandColor": "0xFF5500", "HourColor": "0xFFFFFF",
          "MinuteMarkerColor": "0xAAAAAA", "CompCircleColor": "0xFF5500", "CompFillColor": "0xFF5500",
          "CompTextColor": "0xFFFFFF", "HourStyle": "0", "HourPosition": "0", "MinuteMarkerInterval": 15,
          "MinuteMarkerStyle": "2", "SmartHierarchicalMarkers": true, "Complication1": "3",
          "Complication2": "0", "Complication3": "0", "BTDisconnectPattern": "2",
          "ShowMinuteBubble": false, "HandOverNumbers": false, "ShowMinuteMarkers": true, "HandLengthScreenEdge": true
        };
        Object.keys(defaultSettings).forEach(function(key) {
          var item = clayConfig.getItemByMessageKey(key);
          if (item && item.set) item.set(defaultSettings[key]);
        });
        if (ioBox) ioBox.set("Defaults Restored! Hit Save Settings.");
      });
    }

    if (btnExport) {
      btnExport.on('click', function() {
        var settings = {};
        configKeys.forEach(function(key) {
          var item = clayConfig.getItemByMessageKey(key);
          if (item && item.get) settings[key] = item.get();
        });
        var code = btoa(JSON.stringify(settings));
        if (ioBox) ioBox.set(code);
      });
    }

    if (btnImport) {
      btnImport.on('click', function() {
        try {
          var code = ioBox.get();
          if (!code || code === "") return;
          var settings = JSON.parse(atob(code));
          Object.keys(settings).forEach(function(key) {
            var item = clayConfig.getItemByMessageKey(key);
            if (item && item.set) item.set(settings[key]);
          });
          ioBox.set("SUCCESS! Hit Save Settings to apply.");
        } catch (e) {
          ioBox.set("ERROR: Invalid Theme Code.");
        }
      });
    }
  });
}

var clay = new Clay(clayConfig, customClayJS);