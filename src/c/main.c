#include <pebble.h>

static Window *s_main_window;
static Layer *s_canvas_layer;

// Fonts
static GFont s_time_font_numbers;
static GFont s_time_font_roman;
static GFont s_comp_font_small;
static GFont s_comp_font_medium;
static GFont s_comp_font_large;

#define VIRTUAL_CLOCK_RADIUS 280 
#define CAMERA_RADIUS 230

// --- COLOR STATE VARIABLES ---
static GColor s_bg_color;
static GColor s_minute_hand_color;
static GColor s_hour_color;
static GColor s_marker_color;
static GColor s_comp_circle_color;
static GColor s_comp_text_color;
static GColor s_comp_fill_color;

// --- DYNAMIC THEME ENGINE VARIABLES ---
static int s_theme_mode = 0; // 0=Always Day, 1=Manual, 2=Auto GPS
static GColor s_day_colors[7];
static GColor s_night_colors[7];
static int s_day_h = 7, s_day_m = 0;
static int s_night_h = 19, s_night_m = 0;

// --- BEHAVIOR STATE VARIABLES ---
static bool s_hand_over_numbers;
static bool s_show_minute_markers;
static bool s_hand_length_screen_edge;
static bool s_show_minute_bubble;
static int s_bt_pattern; 

// --- STYLING STATE VARIABLES ---
static int s_hour_style; 
static int s_hour_position; // 0=On Top, 1=Outside, 2=Inside
static int s_marker_interval;
static int s_marker_style; 
static bool s_smart_markers;
static int s_comp_1;
static int s_comp_2;
static int s_comp_3;

// --- ANIMATION STATE VARIABLES ---
static Animation *s_minute_animation = NULL;
static int32_t s_anim_progress = 1000;
static int s_old_hour = -1;
static int s_old_minute = -1;
static int s_new_hour = -1;
static int s_new_minute = -1;

// --- CACHED DATA VARIABLES ---
static char s_hour_texts[12][8];
static char s_minute_bubble_text[4];
static char s_comp_text[3][16];
static GFont s_comp_font[3];
static int s_comp_y_offset[3];
static int s_comp_height[3];
static int s_active_comps[3];
static int s_num_comps = 0;

// --- PRECALCULATED GEOMETRY ---
static GPoint s_hour_offsets[12];
static GPoint s_triangle_centers[12];
static GPoint s_marker_outer_offsets[144]; 
static GPoint s_marker_inner_offsets[144];
static int s_marker_styles[144];
static int s_num_markers = 0;

// GPath definitions for the elegant elongated triangles
static GPath *s_triangle_path_out = NULL;
static GPath *s_triangle_path_in = NULL;
static const GPathInfo TRIANGLE_OUT_INFO = { .num_points = 3, .points = (GPoint []) {{-6, 16}, {6, 16}, {0, -4}} };
static const GPathInfo TRIANGLE_IN_INFO = { .num_points = 3, .points = (GPoint []) {{-6, -4}, {6, -4}, {0, 16}} };

static void compass_handler(CompassHeadingData heading_data) {}

static void update_compass_state() {
  if (s_comp_1 == 14 || s_comp_2 == 14 || s_comp_3 == 14) {
    compass_service_set_heading_filter(TRIG_MAX_ANGLE); 
    compass_service_subscribe(compass_handler);
  } else {
    compass_service_unsubscribe();
  }
}

static void get_roman_numeral(int num, char* buffer) {
  const char* romans[] = {"", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII"};
  if (num >= 1 && num <= 12) strcpy(buffer, romans[num]);
  else strcpy(buffer, "");
}

// --- DYNAMIC THEME SWITCHER LOGIC ---
static void apply_current_theme() {
  if (s_theme_mode == 0) {
    // Single Theme Mode (Always Day Colors)
    s_bg_color = s_day_colors[0]; s_minute_hand_color = s_day_colors[1]; s_hour_color = s_day_colors[2];
    s_marker_color = s_day_colors[3]; s_comp_circle_color = s_day_colors[4]; s_comp_fill_color = s_day_colors[5]; s_comp_text_color = s_day_colors[6];
    return;
  }

  // Calculate if we are in the "Day" window
  int current_minutes = (s_new_hour * 60) + s_new_minute;
  int day_start_minutes = (s_day_h * 60) + s_day_m;
  int night_start_minutes = (s_night_h * 60) + s_night_m;

  bool is_daytime = false;
  if (day_start_minutes < night_start_minutes) {
    is_daytime = (current_minutes >= day_start_minutes && current_minutes < night_start_minutes);
  } else {
    is_daytime = (current_minutes >= day_start_minutes || current_minutes < night_start_minutes);
  }

  if (is_daytime) {
    s_bg_color = s_day_colors[0]; s_minute_hand_color = s_day_colors[1]; s_hour_color = s_day_colors[2];
    s_marker_color = s_day_colors[3]; s_comp_circle_color = s_day_colors[4]; s_comp_fill_color = s_day_colors[5]; s_comp_text_color = s_day_colors[6];
  } else {
    s_bg_color = s_night_colors[0]; s_minute_hand_color = s_night_colors[1]; s_hour_color = s_night_colors[2];
    s_marker_color = s_night_colors[3]; s_comp_circle_color = s_night_colors[4]; s_comp_fill_color = s_night_colors[5]; s_comp_text_color = s_night_colors[6];
  }
}

static void precalculate_geometry() {
  int r_hours = VIRTUAL_CLOCK_RADIUS;
  if (s_hour_position == 1) r_hours = VIRTUAL_CLOCK_RADIUS + 34; // Shift Outside
  else if (s_hour_position == 2) r_hours = VIRTUAL_CLOCK_RADIUS - 34; // Shift Inside

  for (int h = 0; h < 12; h++) {
    int32_t angle = (h * 60) * TRIG_MAX_ANGLE / 720; 
    int32_t sin_a = sin_lookup(angle);
    int32_t cos_a = cos_lookup(angle);
    
    // Positions for the text
    s_hour_offsets[h].x = (sin_a * r_hours) / TRIG_MAX_RATIO;
    s_hour_offsets[h].y = -(cos_a * r_hours) / TRIG_MAX_RATIO;

    // Anchor points for the triangles (always anchored to the track)
    s_triangle_centers[h].x = (sin_a * VIRTUAL_CLOCK_RADIUS) / TRIG_MAX_RATIO;
    s_triangle_centers[h].y = -(cos_a * VIRTUAL_CLOCK_RADIUS) / TRIG_MAX_RATIO;
  }

  s_num_markers = 0;
  if (s_show_minute_markers && s_marker_interval >= 5) {
    for (int m = 0; m < 720; m += s_marker_interval) {
      if (m % 60 == 0) continue; // Skip full hours (either text or triangle goes here)
      
      int32_t angle = m * TRIG_MAX_ANGLE / 720;
      int style = s_marker_style;
      
      if (s_smart_markers) {
        if (m % 30 == 0) style = 3;      
        else if (m % 15 == 0) style = 2; 
        else if (m % 5 == 0) style = 0;  
        else style = -1;                 
      }

      if (style != -1) {
        int32_t r_outer = VIRTUAL_CLOCK_RADIUS;
        int32_t r_inner = (style == 2) ? VIRTUAL_CLOCK_RADIUS - 6 : VIRTUAL_CLOCK_RADIUS - 12;
        
        if (style == 0 || style == 1) r_inner = r_outer - ((style == 0) ? 2 : 4); 

        s_marker_outer_offsets[s_num_markers].x = (sin_lookup(angle) * r_outer) / TRIG_MAX_RATIO;
        s_marker_outer_offsets[s_num_markers].y = -(cos_lookup(angle) * r_outer) / TRIG_MAX_RATIO;
        s_marker_inner_offsets[s_num_markers].x = (sin_lookup(angle) * r_inner) / TRIG_MAX_RATIO;
        s_marker_inner_offsets[s_num_markers].y = -(cos_lookup(angle) * r_inner) / TRIG_MAX_RATIO;
        s_marker_styles[s_num_markers] = style;
        
        s_num_markers++;
      }
    }
  }
}

static void update_cached_data() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  snprintf(s_minute_bubble_text, sizeof(s_minute_bubble_text), "%d", s_new_minute);

  for (int h = 0; h < 12; h++) {
    int display_h = h;
    if (display_h == 0) display_h = 12; 
    
    if (s_hour_style == 1) get_roman_numeral(display_h, s_hour_texts[h]);
    else snprintf(s_hour_texts[h], sizeof(s_hour_texts[h]), "%d", display_h);
  }

  s_num_comps = 0;
  if (s_comp_1 != 0) s_active_comps[s_num_comps++] = s_comp_1;
  if (s_comp_2 != 0) s_active_comps[s_num_comps++] = s_comp_2;
  if (s_comp_3 != 0) s_active_comps[s_num_comps++] = s_comp_3;

  for (int i = 0; i < s_num_comps; i++) {
    s_comp_font[i] = s_comp_font_medium;
    s_comp_y_offset[i] = 14;
    s_comp_height[i] = 32;

    switch(s_active_comps[i]) {
      case 1: { int steps = (int)health_service_sum_today(HealthMetricStepCount); if (steps < 1000) { snprintf(s_comp_text[i], sizeof(s_comp_text[i]), "%d", steps); s_comp_font[i] = s_comp_font_large; s_comp_y_offset[i] = 18; } else if (steps < 10000) { snprintf(s_comp_text[i], sizeof(s_comp_text[i]), "%d.%dk", steps / 1000, (steps % 1000) / 100); } else { snprintf(s_comp_text[i], sizeof(s_comp_text[i]), "%dk", steps / 1000); } break; }
      case 2: strftime(s_comp_text[i], sizeof(s_comp_text[i]), "%d\n%b", tick_time); s_comp_font[i] = s_comp_font_small; s_comp_y_offset[i] = 19; s_comp_height[i] = 40; break;
      case 3: strftime(s_comp_text[i], sizeof(s_comp_text[i]), "%d", tick_time); s_comp_font[i] = s_comp_font_large; s_comp_y_offset[i] = 18; break;
      case 5: snprintf(s_comp_text[i], sizeof(s_comp_text[i]), "%d%%", battery_state_service_peek().charge_percent); break;
      case 6: snprintf(s_comp_text[i], sizeof(s_comp_text[i]), "%d", (int)health_service_peek_current_value(HealthMetricHeartRateBPM)); s_comp_font[i] = s_comp_font_large; s_comp_y_offset[i] = 18; break;
      case 7: strftime(s_comp_text[i], sizeof(s_comp_text[i]), "%a", tick_time); break;
      case 8: strftime(s_comp_text[i], sizeof(s_comp_text[i]), "%p", tick_time); break;
      case 9: strftime(s_comp_text[i], sizeof(s_comp_text[i]), "Wk %V", tick_time); s_comp_font[i] = s_comp_font_small; s_comp_y_offset[i] = 11; break;
      case 10: strftime(s_comp_text[i], sizeof(s_comp_text[i]), "%Y", tick_time); break;
      case 11: { int dist = (int)health_service_sum_today(HealthMetricWalkedDistanceMeters); snprintf(s_comp_text[i], sizeof(s_comp_text[i]), "%d.%d km", dist / 1000, (dist % 1000) / 100); s_comp_font[i] = s_comp_font_small; s_comp_y_offset[i] = 11; break; }
      case 12: { int cal = (int)health_service_sum_today(HealthMetricActiveKCalories); snprintf(s_comp_text[i], sizeof(s_comp_text[i]), "%d kc", cal); s_comp_font[i] = s_comp_font_small; s_comp_y_offset[i] = 11; break; }
      case 13: { int act = (int)health_service_sum_today(HealthMetricActiveSeconds); snprintf(s_comp_text[i], sizeof(s_comp_text[i]), "%d m", act / 60); break; }
      case 14: { CompassHeadingData hdg; compass_service_peek(&hdg); snprintf(s_comp_text[i], sizeof(s_comp_text[i]), "%ld°", TRIGANGLE_TO_DEG(hdg.true_heading)); break; }
    }
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // --- THEME SETTINGS ---
  Tuple *t_mode = dict_find(iter, MESSAGE_KEY_ThemeMode); if(t_mode) { s_theme_mode = atoi(t_mode->value->cstring); persist_write_int(MESSAGE_KEY_ThemeMode, s_theme_mode); }

// Extract Theme Times (JS now handles API injection, so the watch just reads these fields)
  Tuple *t_man_day = dict_find(iter, MESSAGE_KEY_ManualDayTime); 
  if(t_man_day && strlen(t_man_day->value->cstring) >= 5) {
      char *s = t_man_day->value->cstring;
      int h = (s[0]-'0')*10 + (s[1]-'0'); int m = (s[3]-'0')*10 + (s[4]-'0');
      persist_write_int(1001, h); persist_write_int(1002, m);
      if(s_theme_mode == 1 || s_theme_mode == 2) { s_day_h = h; s_day_m = m; }
  }
  
  Tuple *t_man_night = dict_find(iter, MESSAGE_KEY_ManualNightTime); 
  if(t_man_night && strlen(t_man_night->value->cstring) >= 5) {
      char *s = t_man_night->value->cstring;
      int h = (s[0]-'0')*10 + (s[1]-'0'); int m = (s[3]-'0')*10 + (s[4]-'0');
      persist_write_int(1003, h); persist_write_int(1004, m);
      if(s_theme_mode == 1 || s_theme_mode == 2) { s_night_h = h; s_night_m = m; }
  }

  // --- DAY COLORS ---
  Tuple *c1 = dict_find(iter, MESSAGE_KEY_BackgroundColor); if(c1) { s_day_colors[0] = GColorFromHEX(c1->value->int32); persist_write_int(MESSAGE_KEY_BackgroundColor, s_day_colors[0].argb); }
  Tuple *c2 = dict_find(iter, MESSAGE_KEY_MinuteHandColor); if(c2) { s_day_colors[1] = GColorFromHEX(c2->value->int32); persist_write_int(MESSAGE_KEY_MinuteHandColor, s_day_colors[1].argb); }
  Tuple *c3 = dict_find(iter, MESSAGE_KEY_HourColor); if(c3) { s_day_colors[2] = GColorFromHEX(c3->value->int32); persist_write_int(MESSAGE_KEY_HourColor, s_day_colors[2].argb); }
  Tuple *c4 = dict_find(iter, MESSAGE_KEY_MinuteMarkerColor); if(c4) { s_day_colors[3] = GColorFromHEX(c4->value->int32); persist_write_int(MESSAGE_KEY_MinuteMarkerColor, s_day_colors[3].argb); }
  Tuple *c5 = dict_find(iter, MESSAGE_KEY_CompCircleColor); if(c5) { s_day_colors[4] = GColorFromHEX(c5->value->int32); persist_write_int(MESSAGE_KEY_CompCircleColor, s_day_colors[4].argb); }
  Tuple *c6 = dict_find(iter, MESSAGE_KEY_CompFillColor); if(c6) { s_day_colors[5] = GColorFromHEX(c6->value->int32); persist_write_int(MESSAGE_KEY_CompFillColor, s_day_colors[5].argb); }
  Tuple *c7 = dict_find(iter, MESSAGE_KEY_CompTextColor); if(c7) { s_day_colors[6] = GColorFromHEX(c7->value->int32); persist_write_int(MESSAGE_KEY_CompTextColor, s_day_colors[6].argb); }

  // --- NIGHT COLORS ---
  Tuple *n1 = dict_find(iter, MESSAGE_KEY_NightBackgroundColor); if(n1) { s_night_colors[0] = GColorFromHEX(n1->value->int32); persist_write_int(MESSAGE_KEY_NightBackgroundColor, s_night_colors[0].argb); }
  Tuple *n2 = dict_find(iter, MESSAGE_KEY_NightMinuteHandColor); if(n2) { s_night_colors[1] = GColorFromHEX(n2->value->int32); persist_write_int(MESSAGE_KEY_NightMinuteHandColor, s_night_colors[1].argb); }
  Tuple *n3 = dict_find(iter, MESSAGE_KEY_NightHourColor); if(n3) { s_night_colors[2] = GColorFromHEX(n3->value->int32); persist_write_int(MESSAGE_KEY_NightHourColor, s_night_colors[2].argb); }
  Tuple *n4 = dict_find(iter, MESSAGE_KEY_NightMinuteMarkerColor); if(n4) { s_night_colors[3] = GColorFromHEX(n4->value->int32); persist_write_int(MESSAGE_KEY_NightMinuteMarkerColor, s_night_colors[3].argb); }
  Tuple *n5 = dict_find(iter, MESSAGE_KEY_NightCompCircleColor); if(n5) { s_night_colors[4] = GColorFromHEX(n5->value->int32); persist_write_int(MESSAGE_KEY_NightCompCircleColor, s_night_colors[4].argb); }
  Tuple *n6 = dict_find(iter, MESSAGE_KEY_NightCompFillColor); if(n6) { s_night_colors[5] = GColorFromHEX(n6->value->int32); persist_write_int(MESSAGE_KEY_NightCompFillColor, s_night_colors[5].argb); }
  Tuple *n7 = dict_find(iter, MESSAGE_KEY_NightCompTextColor); if(n7) { s_night_colors[6] = GColorFromHEX(n7->value->int32); persist_write_int(MESSAGE_KEY_NightCompTextColor, s_night_colors[6].argb); }

  // --- STANDARD STYLES ---
  Tuple *hand_over_t = dict_find(iter, MESSAGE_KEY_HandOverNumbers); if (hand_over_t) { s_hand_over_numbers = hand_over_t->value->int32 == 1 || hand_over_t->value->uint8 == 1; persist_write_bool(MESSAGE_KEY_HandOverNumbers, s_hand_over_numbers); }
  Tuple *show_markers_t = dict_find(iter, MESSAGE_KEY_ShowMinuteMarkers); if (show_markers_t) { s_show_minute_markers = show_markers_t->value->int32 == 1 || show_markers_t->value->uint8 == 1; persist_write_bool(MESSAGE_KEY_ShowMinuteMarkers, s_show_minute_markers); }
  Tuple *hand_length_t = dict_find(iter, MESSAGE_KEY_HandLengthScreenEdge); if (hand_length_t) { s_hand_length_screen_edge = hand_length_t->value->int32 == 1 || hand_length_t->value->uint8 == 1; persist_write_bool(MESSAGE_KEY_HandLengthScreenEdge, s_hand_length_screen_edge); }
  Tuple *show_bubble_t = dict_find(iter, MESSAGE_KEY_ShowMinuteBubble); if (show_bubble_t) { s_show_minute_bubble = show_bubble_t->value->int32 == 1 || show_bubble_t->value->uint8 == 1; persist_write_bool(MESSAGE_KEY_ShowMinuteBubble, s_show_minute_bubble); }

  Tuple *hour_style_t = dict_find(iter, MESSAGE_KEY_HourStyle); if (hour_style_t) { s_hour_style = atoi(hour_style_t->value->cstring); persist_write_int(MESSAGE_KEY_HourStyle, s_hour_style); }
  Tuple *hour_pos_t = dict_find(iter, MESSAGE_KEY_HourPosition); if (hour_pos_t) { s_hour_position = atoi(hour_pos_t->value->cstring); persist_write_int(MESSAGE_KEY_HourPosition, s_hour_position); }
  Tuple *interval_t = dict_find(iter, MESSAGE_KEY_MinuteMarkerInterval); if (interval_t) { s_marker_interval = interval_t->value->int32; persist_write_int(MESSAGE_KEY_MinuteMarkerInterval, s_marker_interval); }
  Tuple *marker_style_t = dict_find(iter, MESSAGE_KEY_MinuteMarkerStyle); if (marker_style_t) { s_marker_style = atoi(marker_style_t->value->cstring); persist_write_int(MESSAGE_KEY_MinuteMarkerStyle, s_marker_style); }
  Tuple *smart_markers_t = dict_find(iter, MESSAGE_KEY_SmartHierarchicalMarkers); if (smart_markers_t) { s_smart_markers = smart_markers_t->value->int32 == 1 || smart_markers_t->value->uint8 == 1; persist_write_bool(MESSAGE_KEY_SmartHierarchicalMarkers, s_smart_markers); }
  Tuple *bt_t = dict_find(iter, MESSAGE_KEY_BTDisconnectPattern); if (bt_t) { s_bt_pattern = atoi(bt_t->value->cstring); persist_write_int(MESSAGE_KEY_BTDisconnectPattern, s_bt_pattern); }
  
  Tuple *c1_t = dict_find(iter, MESSAGE_KEY_Complication1); if (c1_t) { s_comp_1 = atoi(c1_t->value->cstring); persist_write_int(MESSAGE_KEY_Complication1, s_comp_1); }
  Tuple *c2_t = dict_find(iter, MESSAGE_KEY_Complication2); if (c2_t) { s_comp_2 = atoi(c2_t->value->cstring); persist_write_int(MESSAGE_KEY_Complication2, s_comp_2); }
  Tuple *c3_t = dict_find(iter, MESSAGE_KEY_Complication3); if (c3_t) { s_comp_3 = atoi(c3_t->value->cstring); persist_write_int(MESSAGE_KEY_Complication3, s_comp_3); }

  update_compass_state();
  precalculate_geometry(); 
  
  // Apply Dynamic Theme Logic
  apply_current_theme(); 
  
  update_cached_data();
  layer_mark_dirty(s_canvas_layer);
}

static void anim_update_proc(Animation *animation, AnimationProgress progress) {
  s_anim_progress = ((int32_t)progress * 1000) / ANIMATION_NORMALIZED_MAX;
  layer_mark_dirty(s_canvas_layer);
}

static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
  animation_destroy(animation);
  s_minute_animation = NULL; 
}

static AnimationImplementation s_anim_impl = { .update = anim_update_proc };

static void start_minute_animation() {
  if (s_minute_animation) { animation_unschedule(s_minute_animation); }
  s_minute_animation = animation_create();
  animation_set_duration(s_minute_animation, 800); 
  animation_set_curve(s_minute_animation, AnimationCurveEaseInOut); 
  animation_set_implementation(s_minute_animation, &s_anim_impl);
  animation_set_handlers(s_minute_animation, (AnimationHandlers) { .stopped = anim_stopped_handler }, NULL);
  animation_schedule(s_minute_animation);
}

static void draw_minute_hand(GContext *ctx, int32_t cx, int32_t cy, int32_t sin_m, int32_t cos_m) {
  int32_t line_length_end = s_hand_length_screen_edge ? (VIRTUAL_CLOCK_RADIUS + 150) : VIRTUAL_CLOCK_RADIUS;
  int32_t line_length_start = VIRTUAL_CLOCK_RADIUS + 150; 

  GPoint p0 = { .x = cx - (sin_m * line_length_start) / TRIG_MAX_RATIO, .y = cy + (cos_m * line_length_start) / TRIG_MAX_RATIO };
  GPoint p1 = { .x = cx + (sin_m * line_length_end) / TRIG_MAX_RATIO,   .y = cy - (cos_m * line_length_end) / TRIG_MAX_RATIO };

  graphics_context_set_stroke_color(ctx, s_minute_hand_color);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_context_set_antialiased(ctx, true);
  graphics_draw_line(ctx, p0, p1);

  if (s_show_minute_bubble) {
    GPoint bubble_center = GPoint(cx + (sin_m * VIRTUAL_CLOCK_RADIUS) / TRIG_MAX_RATIO, cy - (cos_m * VIRTUAL_CLOCK_RADIUS) / TRIG_MAX_RATIO);
    int bubble_radius = 16;
    
    graphics_context_set_fill_color(ctx, s_bg_color);
    graphics_fill_circle(ctx, bubble_center, bubble_radius);
    graphics_context_set_stroke_color(ctx, s_minute_hand_color);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_circle(ctx, bubble_center, bubble_radius);
    
    GRect min_rect = GRect(bubble_center.x - bubble_radius, bubble_center.y - 13, bubble_radius * 2, 22);
    graphics_context_set_text_color(ctx, s_minute_hand_color);
    graphics_draw_text(ctx, s_minute_bubble_text, s_comp_font_medium, min_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, s_bg_color);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  GPoint center = grect_center_point(&bounds);
  int cull_w = bounds.size.w + 40;
  int cull_h = bounds.size.h + 40;

  int32_t old_h_12 = s_old_hour % 12;
  int32_t new_h_12 = s_new_hour % 12;
  int32_t old_angle = ((old_h_12 * 60) + s_old_minute) * TRIG_MAX_ANGLE / 720;
  int32_t new_angle = ((new_h_12 * 60) + s_new_minute) * TRIG_MAX_ANGLE / 720;
  
  if (old_h_12 == 11 && new_h_12 == 0) new_angle += TRIG_MAX_ANGLE;
  else if (old_h_12 == 0 && new_h_12 == 11) old_angle += TRIG_MAX_ANGLE;

  int32_t interpolated_angle = old_angle + ((new_angle - old_angle) * s_anim_progress) / 1000;
  int32_t minute_angle = interpolated_angle % TRIG_MAX_ANGLE;

  int32_t sin_m = sin_lookup(minute_angle);
  int32_t cos_m = cos_lookup(minute_angle);

  int32_t cx = center.x - (sin_m * CAMERA_RADIUS) / TRIG_MAX_RATIO;
  int32_t cy = center.y - (-cos_m * CAMERA_RADIUS) / TRIG_MAX_RATIO;

  if (!s_hand_over_numbers) draw_minute_hand(ctx, cx, cy, sin_m, cos_m);

  // --- DRAW FAST-CACHED HOURS ---
  graphics_context_set_text_color(ctx, s_hour_color);
  GFont active_font = (s_hour_style == 1) ? s_time_font_roman : s_time_font_numbers;
  
  for (int h = 0; h < 12; h++) {
    int32_t text_x = cx + s_hour_offsets[h].x;
    int32_t text_y = cy + s_hour_offsets[h].y;
    
    if (text_x < -40 || text_x > cull_w || text_y < -40 || text_y > cull_h) continue;
    
    GRect text_rect = GRect(text_x - 30, text_y - 20, 60, 40);
    graphics_draw_text(ctx, s_hour_texts[h], active_font, text_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }

  // --- DRAW ELONGATED TRIANGLE HOUR MARKERS ---
  if (s_hour_position != 0) {
    graphics_context_set_fill_color(ctx, s_marker_color);
    GPath *active_path = s_triangle_path_in;
    
    for (int h = 0; h < 12; h++) {
      GPoint pt = { cx + s_triangle_centers[h].x, cy + s_triangle_centers[h].y };
      if (pt.x < -40 || pt.x > cull_w || pt.y < -40 || pt.y > cull_h) continue;
      
      gpath_move_to(active_path, pt);
      gpath_rotate_to(active_path, (h * 60) * TRIG_MAX_ANGLE / 720);
      gpath_draw_filled(ctx, active_path);
    }
  }

  // --- DRAW FAST-CACHED MINUTE MARKERS ---
  if (s_num_markers > 0) {
    graphics_context_set_stroke_color(ctx, s_marker_color);
    graphics_context_set_fill_color(ctx, s_marker_color);
    graphics_context_set_stroke_width(ctx, 2);
    
    for (int i = 0; i < s_num_markers; i++) {
      GPoint p_outer = { cx + s_marker_outer_offsets[i].x, cy + s_marker_outer_offsets[i].y };
      if (p_outer.x < -20 || p_outer.x > cull_w || p_outer.y < -20 || p_outer.y > cull_h) continue;

      GPoint p_inner = { cx + s_marker_inner_offsets[i].x, cy + s_marker_inner_offsets[i].y };
      int style = s_marker_styles[i];

      if (style == 2 || style == 3) {
        graphics_draw_line(ctx, p_outer, p_inner);
      } else {
        int radius = (style == 0) ? 2 : 4;
        graphics_fill_circle(ctx, p_inner, radius);
      }
    }
  }

  if (s_hand_over_numbers) draw_minute_hand(ctx, cx, cy, sin_m, cos_m);

  // --- DRAW COMPLICATIONS ---
  if (s_num_comps > 0) {
    int comp_radius = 26;
    int inner_radius = comp_radius - 4; 
    int comp_spacing = 40; 
    int pill_offset = -50; 

    int start_offset = - ((s_num_comps - 1) * comp_spacing / 2);
    int end_offset = ((s_num_comps - 1) * comp_spacing / 2);

    GPoint p_start = { .x = center.x + (sin_m * (pill_offset + start_offset)) / TRIG_MAX_RATIO, .y = center.y - (cos_m * (pill_offset + start_offset)) / TRIG_MAX_RATIO };
    GPoint p_end = { .x = center.x + (sin_m * (pill_offset + end_offset)) / TRIG_MAX_RATIO, .y = center.y - (cos_m * (pill_offset + end_offset)) / TRIG_MAX_RATIO };

    graphics_context_set_fill_color(ctx, s_bg_color);
    graphics_fill_circle(ctx, p_start, comp_radius);
    graphics_fill_circle(ctx, p_end, comp_radius);
    graphics_context_set_stroke_color(ctx, s_bg_color);
    graphics_context_set_stroke_width(ctx, comp_radius * 2);
    if (s_num_comps > 1) graphics_draw_line(ctx, p_start, p_end);

    graphics_context_set_fill_color(ctx, s_comp_circle_color);
    graphics_fill_circle(ctx, p_start, comp_radius);
    graphics_fill_circle(ctx, p_end, comp_radius);
    graphics_context_set_stroke_color(ctx, s_comp_circle_color);
    graphics_context_set_stroke_width(ctx, comp_radius * 2);
    if (s_num_comps > 1) graphics_draw_line(ctx, p_start, p_end);

    graphics_context_set_fill_color(ctx, s_bg_color);
    graphics_fill_circle(ctx, p_start, comp_radius - 2);
    graphics_fill_circle(ctx, p_end, comp_radius - 2);
    graphics_context_set_stroke_color(ctx, s_bg_color);
    graphics_context_set_stroke_width(ctx, (comp_radius - 2) * 2);
    if (s_num_comps > 1) graphics_draw_line(ctx, p_start, p_end);

    graphics_context_set_fill_color(ctx, s_comp_fill_color);
    graphics_fill_circle(ctx, p_start, inner_radius);
    graphics_fill_circle(ctx, p_end, inner_radius);
    graphics_context_set_stroke_color(ctx, s_comp_fill_color);
    graphics_context_set_stroke_width(ctx, inner_radius * 2);
    if (s_num_comps > 1) graphics_draw_line(ctx, p_start, p_end);

    if (s_num_comps > 1) {
      int32_t perp_angle = minute_angle + (TRIG_MAX_ANGLE / 4);
      int32_t sin_p = sin_lookup(perp_angle);
      int32_t cos_p = cos_lookup(perp_angle);

      graphics_context_set_stroke_color(ctx, s_bg_color);
      graphics_context_set_stroke_width(ctx, 3); 

      for (int i = 0; i < s_num_comps - 1; i++) {
        int sep_offset = start_offset + (i * comp_spacing) + (comp_spacing / 2);
        int32_t sep_cx = center.x + (sin_m * (pill_offset + sep_offset)) / TRIG_MAX_RATIO;
        int32_t sep_cy = center.y - (cos_m * (pill_offset + sep_offset)) / TRIG_MAX_RATIO;

        int cut_length = inner_radius + 2; 
        GPoint p_cut1 = { .x = sep_cx + (sin_p * cut_length) / TRIG_MAX_RATIO, .y = sep_cy - (cos_p * cut_length) / TRIG_MAX_RATIO };
        GPoint p_cut2 = { .x = sep_cx - (sin_p * cut_length) / TRIG_MAX_RATIO, .y = sep_cy + (cos_p * cut_length) / TRIG_MAX_RATIO };
        graphics_draw_line(ctx, p_cut1, p_cut2);
      }
    }

    for (int i = 0; i < s_num_comps; i++) {
      int offset = (i * comp_spacing) + start_offset; 
      GPoint comp_center = {
        .x = center.x + (sin_m * (pill_offset + offset)) / TRIG_MAX_RATIO,
        .y = center.y - (cos_m * (pill_offset + offset)) / TRIG_MAX_RATIO
      };
      GRect text_rect = GRect(comp_center.x - comp_radius, comp_center.y - s_comp_y_offset[i], comp_radius * 2, s_comp_height[i]);
      graphics_context_set_text_color(ctx, s_comp_text_color); 
      graphics_draw_text(ctx, s_comp_text[i], s_comp_font[i], text_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_old_hour = s_new_hour;
  s_old_minute = s_new_minute;
  s_new_hour = tick_time->tm_hour;
  s_new_minute = tick_time->tm_min;
  
  // Re-evaluate if we need to swap day/night colors on the minute tick
  apply_current_theme();
  
  update_cached_data();
  start_minute_animation();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_time_font_numbers = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
  s_time_font_roman = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  s_comp_font_small = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD); 
  s_comp_font_medium = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD); 
  s_comp_font_large = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD); 

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void main_window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}

static void bluetooth_callback(bool connected) {
  if (!connected) {
    switch(s_bt_pattern) {
      case 1: vibes_short_pulse(); break;
      case 2: vibes_double_pulse(); break;
      case 3: vibes_long_pulse(); break;
      default: break; 
    }
  }
}

static void init() {
  s_main_window = window_create();
  
// Initialize Theme Logic
  s_theme_mode = persist_exists(MESSAGE_KEY_ThemeMode) ? persist_read_int(MESSAGE_KEY_ThemeMode) : 0;
  
  // Both Manual (1) and Auto API (2) now use the same saved time slots!
  if (s_theme_mode == 1 || s_theme_mode == 2) { 
      if(persist_exists(1001)) s_day_h = persist_read_int(1001);
      if(persist_exists(1002)) s_day_m = persist_read_int(1002);
      if(persist_exists(1003)) s_night_h = persist_read_int(1003);
      if(persist_exists(1004)) s_night_m = persist_read_int(1004);
  }

  // Load Day Colors
  s_day_colors[0].argb = persist_exists(MESSAGE_KEY_BackgroundColor) ? persist_read_int(MESSAGE_KEY_BackgroundColor) : GColorBlackARGB8;
  s_day_colors[1].argb = persist_exists(MESSAGE_KEY_MinuteHandColor) ? persist_read_int(MESSAGE_KEY_MinuteHandColor) : GColorOrangeARGB8;
  s_day_colors[2].argb = persist_exists(MESSAGE_KEY_HourColor) ? persist_read_int(MESSAGE_KEY_HourColor) : GColorWhiteARGB8;
  s_day_colors[3].argb = persist_exists(MESSAGE_KEY_MinuteMarkerColor) ? persist_read_int(MESSAGE_KEY_MinuteMarkerColor) : GColorLightGrayARGB8;
  s_day_colors[4].argb = persist_exists(MESSAGE_KEY_CompCircleColor) ? persist_read_int(MESSAGE_KEY_CompCircleColor) : GColorOrangeARGB8;
  s_day_colors[5].argb = persist_exists(MESSAGE_KEY_CompFillColor) ? persist_read_int(MESSAGE_KEY_CompFillColor) : GColorOrangeARGB8;
  s_day_colors[6].argb = persist_exists(MESSAGE_KEY_CompTextColor) ? persist_read_int(MESSAGE_KEY_CompTextColor) : GColorWhiteARGB8;

  // Load Night Colors
  s_night_colors[0].argb = persist_exists(MESSAGE_KEY_NightBackgroundColor) ? persist_read_int(MESSAGE_KEY_NightBackgroundColor) : GColorBlackARGB8;
  s_night_colors[1].argb = persist_exists(MESSAGE_KEY_NightMinuteHandColor) ? persist_read_int(MESSAGE_KEY_NightMinuteHandColor) : GColorCobaltBlueARGB8;
  s_night_colors[2].argb = persist_exists(MESSAGE_KEY_NightHourColor) ? persist_read_int(MESSAGE_KEY_NightHourColor) : GColorLightGrayARGB8;
  s_night_colors[3].argb = persist_exists(MESSAGE_KEY_NightMinuteMarkerColor) ? persist_read_int(MESSAGE_KEY_NightMinuteMarkerColor) : GColorDarkGrayARGB8;
  s_night_colors[4].argb = persist_exists(MESSAGE_KEY_NightCompCircleColor) ? persist_read_int(MESSAGE_KEY_NightCompCircleColor) : GColorCobaltBlueARGB8;
  s_night_colors[5].argb = persist_exists(MESSAGE_KEY_NightCompFillColor) ? persist_read_int(MESSAGE_KEY_NightCompFillColor) : GColorCobaltBlueARGB8;
  s_night_colors[6].argb = persist_exists(MESSAGE_KEY_NightCompTextColor) ? persist_read_int(MESSAGE_KEY_NightCompTextColor) : GColorLightGrayARGB8;

  s_hand_over_numbers = persist_exists(MESSAGE_KEY_HandOverNumbers) ? persist_read_bool(MESSAGE_KEY_HandOverNumbers) : false;
  s_show_minute_markers = persist_exists(MESSAGE_KEY_ShowMinuteMarkers) ? persist_read_bool(MESSAGE_KEY_ShowMinuteMarkers) : true;
  s_hand_length_screen_edge = persist_exists(MESSAGE_KEY_HandLengthScreenEdge) ? persist_read_bool(MESSAGE_KEY_HandLengthScreenEdge) : true;
  s_show_minute_bubble = persist_exists(MESSAGE_KEY_ShowMinuteBubble) ? persist_read_bool(MESSAGE_KEY_ShowMinuteBubble) : false; 
  
  s_hour_style = persist_exists(MESSAGE_KEY_HourStyle) ? persist_read_int(MESSAGE_KEY_HourStyle) : 0;
  s_hour_position = persist_exists(MESSAGE_KEY_HourPosition) ? persist_read_int(MESSAGE_KEY_HourPosition) : 0;
  s_marker_interval = persist_exists(MESSAGE_KEY_MinuteMarkerInterval) ? persist_read_int(MESSAGE_KEY_MinuteMarkerInterval) : 15; 
  s_marker_style = persist_exists(MESSAGE_KEY_MinuteMarkerStyle) ? persist_read_int(MESSAGE_KEY_MinuteMarkerStyle) : 2;
  s_smart_markers = persist_exists(MESSAGE_KEY_SmartHierarchicalMarkers) ? persist_read_bool(MESSAGE_KEY_SmartHierarchicalMarkers) : true; 
  
  s_comp_1 = persist_exists(MESSAGE_KEY_Complication1) ? persist_read_int(MESSAGE_KEY_Complication1) : 3; 
  s_comp_2 = persist_exists(MESSAGE_KEY_Complication2) ? persist_read_int(MESSAGE_KEY_Complication2) : 0; 
  s_comp_3 = persist_exists(MESSAGE_KEY_Complication3) ? persist_read_int(MESSAGE_KEY_Complication3) : 0; 

  // Initialize triangle graphics
  s_triangle_path_out = gpath_create(&TRIANGLE_OUT_INFO);
  s_triangle_path_in = gpath_create(&TRIANGLE_IN_INFO);

  window_set_background_color(s_main_window, s_bg_color);

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(512, 512); 

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true);
  
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  s_new_hour = tick_time->tm_hour;
  s_new_minute = tick_time->tm_min;
  s_old_hour = s_new_hour;
  s_old_minute = s_new_minute;
  s_anim_progress = 1000; 

  update_compass_state();
  precalculate_geometry(); 
  apply_current_theme(); // Apply the theme BEFORE first draw
  update_cached_data();

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  s_bt_pattern = persist_exists(MESSAGE_KEY_BTDisconnectPattern) ? persist_read_int(MESSAGE_KEY_BTDisconnectPattern) : 2;
  connection_service_subscribe((ConnectionHandlers) { .pebble_app_connection_handler = bluetooth_callback });
}

static void deinit() {
  if (s_minute_animation) animation_destroy(s_minute_animation); 
  
  // Clean up graphics objects
  gpath_destroy(s_triangle_path_out);
  gpath_destroy(s_triangle_path_in);
  
  window_destroy(s_main_window);
  connection_service_unsubscribe();
  compass_service_unsubscribe(); 
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}