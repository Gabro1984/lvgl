/**
 * @file lv_meter.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_meter.h"
#if LV_USE_METER != 0

#include "../../misc/lv_assert.h"
#include "../../misc/lv_color.h"
#include "../../stdlib/lv_sprintf.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_meter_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_meter_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_meter_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_meter_event(const lv_obj_class_t * class_p, lv_event_t * e);
static void draw_arcs(lv_obj_t * obj, lv_layer_t * layer, const lv_area_t * scale_area);
static void draw_ticks_and_labels(lv_obj_t * obj, lv_layer_t * layer, const lv_area_t * scale_area);
static void draw_needles(lv_obj_t * obj, lv_layer_t * layer, const lv_area_t * scale_area);
static void inv_arc(lv_obj_t * obj, lv_meter_indicator_t * indic, int32_t old_value, int32_t new_value);
static void inv_line(lv_obj_t * obj, lv_meter_indicator_t * indic, int32_t value);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_meter_class = {
    .constructor_cb = lv_meter_constructor,
    .destructor_cb = lv_meter_destructor,
    .event_cb = lv_meter_event,
    .instance_size = sizeof(lv_meter_t),
    .base_class = &lv_obj_class
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_meter_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*=====================
 * Add scale
 *====================*/

void lv_meter_set_scale_ticks(lv_obj_t * obj, uint16_t cnt, uint16_t width, uint16_t len,
                              lv_color_t color)
{
    lv_meter_t * meter = (lv_meter_t *)obj;
    meter->scale.tick_cnt = cnt;
    meter->scale.tick_width = width;
    meter->scale.tick_length = len;
    meter->scale.tick_color = color;
    lv_obj_invalidate(obj);
}

void lv_meter_set_scale_major_ticks(lv_obj_t * obj,  uint16_t nth, uint16_t width,
                                    uint16_t len, lv_color_t color, int16_t label_gap)
{
    lv_meter_t * meter = (lv_meter_t *)obj;
    meter->scale.tick_major_nth = nth;
    meter->scale.tick_major_width = width;
    meter->scale.tick_major_length = len;
    meter->scale.tick_major_color = color;
    meter->scale.label_gap = label_gap;
    lv_obj_invalidate(obj);
}

void lv_meter_set_scale_range(lv_obj_t * obj, int32_t min, int32_t max, uint32_t angle_range,
                              uint32_t rotation)
{
    lv_meter_t * meter = (lv_meter_t *)obj;
    meter->scale.min = min;
    meter->scale.max = max;
    meter->scale.angle_range = angle_range;
    meter->scale.rotation = rotation;
    lv_obj_invalidate(obj);
}

/*=====================
 * Add indicator
 *====================*/

lv_meter_indicator_t * lv_meter_add_needle_line(lv_obj_t * obj, uint16_t width,
                                                lv_color_t color, int16_t r_mod)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_meter_t * meter = (lv_meter_t *)obj;
    lv_meter_indicator_t * indic = _lv_ll_ins_head(&meter->indicator_ll);
    LV_ASSERT_MALLOC(indic);
    if(NULL == indic) {
        return NULL;
    }

    lv_memzero(indic, sizeof(lv_meter_indicator_t));
    indic->opa = LV_OPA_COVER;

    indic->type = LV_METER_INDICATOR_TYPE_NEEDLE_LINE;
    indic->type_data.needle_line.width = width;
    indic->type_data.needle_line.color = color;
    indic->type_data.needle_line.r_mod = r_mod;
    lv_obj_invalidate(obj);

    return indic;
}

lv_meter_indicator_t * lv_meter_add_needle_image(lv_obj_t * obj, const void * src,
                                                 lv_coord_t pivot_x, lv_coord_t pivot_y)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_meter_t * meter = (lv_meter_t *)obj;
    lv_meter_indicator_t * indic = _lv_ll_ins_head(&meter->indicator_ll);
    LV_ASSERT_MALLOC(indic);
    if(NULL == indic) {
        return NULL;
    }

    lv_memzero(indic, sizeof(lv_meter_indicator_t));
    indic->opa = LV_OPA_COVER;

    indic->type = LV_METER_INDICATOR_TYPE_NEEDLE_IMG;
    indic->type_data.needle_img.src = src;
    indic->type_data.needle_img.pivot.x = pivot_x;
    indic->type_data.needle_img.pivot.y = pivot_y;
    lv_obj_invalidate(obj);

    return indic;
}

lv_meter_indicator_t * lv_meter_add_arc(lv_obj_t * obj, uint16_t width, lv_color_t color,
                                        int16_t r_mod)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_meter_t * meter = (lv_meter_t *)obj;
    lv_meter_indicator_t * indic = _lv_ll_ins_head(&meter->indicator_ll);
    LV_ASSERT_MALLOC(indic);
    if(NULL == indic) {
        return NULL;
    }

    lv_memzero(indic, sizeof(lv_meter_indicator_t));
    indic->opa = LV_OPA_COVER;

    indic->type = LV_METER_INDICATOR_TYPE_ARC;
    indic->type_data.arc.width = width;
    indic->type_data.arc.color = color;
    indic->type_data.arc.r_mod = r_mod;

    lv_obj_invalidate(obj);
    return indic;
}

lv_meter_indicator_t * lv_meter_add_scale_lines(lv_obj_t * obj, lv_color_t color_start,
                                                lv_color_t color_end, bool local, int16_t width_mod)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_meter_t * meter = (lv_meter_t *)obj;
    lv_meter_indicator_t * indic = _lv_ll_ins_head(&meter->indicator_ll);
    LV_ASSERT_MALLOC(indic);
    if(NULL == indic) {
        return NULL;
    }

    lv_memzero(indic, sizeof(lv_meter_indicator_t));
    indic->opa = LV_OPA_COVER;

    indic->type = LV_METER_INDICATOR_TYPE_SCALE_LINES;
    indic->type_data.scale_lines.color_start = color_start;
    indic->type_data.scale_lines.color_end = color_end;
    indic->type_data.scale_lines.local_grad = local;
    indic->type_data.scale_lines.width_mod = width_mod;

    lv_obj_invalidate(obj);
    return indic;
}

/*=====================
 * Set indicator value
 *====================*/

void lv_meter_set_indicator_value(lv_obj_t * obj, lv_meter_indicator_t * indic, int32_t value)
{
    int32_t old_start = indic->start_value;
    int32_t old_end = indic->end_value;
    indic->start_value = value;
    indic->end_value = value;

    if(indic->type == LV_METER_INDICATOR_TYPE_ARC) {
        inv_arc(obj, indic, old_start, value);
        inv_arc(obj, indic, old_end, value);
    }
    else if(indic->type == LV_METER_INDICATOR_TYPE_NEEDLE_IMG || indic->type == LV_METER_INDICATOR_TYPE_NEEDLE_LINE) {
        inv_line(obj, indic, old_start);
        inv_line(obj, indic, old_end);
        inv_line(obj, indic, value);
    }
    else {
        lv_obj_invalidate(obj);
    }
}

void lv_meter_set_indicator_start_value(lv_obj_t * obj, lv_meter_indicator_t * indic, int32_t value)
{
    int32_t old_value = indic->start_value;
    indic->start_value = value;

    if(indic->type == LV_METER_INDICATOR_TYPE_ARC) {
        inv_arc(obj, indic, old_value, value);
    }
    else if(indic->type == LV_METER_INDICATOR_TYPE_NEEDLE_IMG || indic->type == LV_METER_INDICATOR_TYPE_NEEDLE_LINE) {
        inv_line(obj, indic, old_value);
        inv_line(obj, indic, value);
    }
    else {
        lv_obj_invalidate(obj);
    }
}

void lv_meter_set_indicator_end_value(lv_obj_t * obj, lv_meter_indicator_t * indic, int32_t value)
{
    int32_t old_value = indic->end_value;
    indic->end_value = value;

    if(indic->type == LV_METER_INDICATOR_TYPE_ARC) {
        inv_arc(obj, indic, old_value, value);
    }
    else if(indic->type == LV_METER_INDICATOR_TYPE_NEEDLE_IMG || indic->type == LV_METER_INDICATOR_TYPE_NEEDLE_LINE) {
        inv_line(obj, indic, old_value);
        inv_line(obj, indic, value);
    }
    else {
        lv_obj_invalidate(obj);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_meter_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_meter_t * meter = (lv_meter_t *)obj;

    _lv_ll_init(&meter->indicator_ll, sizeof(lv_meter_indicator_t));

    meter->scale.angle_range = 270;
    meter->scale.rotation = 90 + (360 - meter->scale.angle_range) / 2;
    meter->scale.min = 0;
    meter->scale.max = 100;
    meter->scale.tick_cnt = 6;
    meter->scale.tick_length = 8;
    meter->scale.tick_width = 2;
    meter->scale.label_gap = 2;


    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_meter_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_meter_t * meter = (lv_meter_t *)obj;

    lv_meter_indicator_t * indic;
    _LV_LL_READ_BACK(&meter->indicator_ll, indic) {
        lv_anim_del(indic, NULL);
    }

    _lv_ll_clear(&meter->indicator_ll);
}

static void lv_meter_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);

    lv_result_t res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RESULT_OK) return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_DRAW_MAIN) {
        lv_layer_t * layer = lv_event_get_layer(e);
        lv_area_t scale_area;
        lv_obj_get_content_coords(obj, &scale_area);

        draw_arcs(obj, layer, &scale_area);
        draw_ticks_and_labels(obj, layer, &scale_area);
        draw_needles(obj, layer, &scale_area);

        lv_coord_t r_edge = lv_area_get_width(&scale_area) / 2;
        lv_point_t scale_center;
        scale_center.x = scale_area.x1 + r_edge;
        scale_center.y = scale_area.y1 + r_edge;

        lv_draw_rect_dsc_t mid_dsc;
        lv_draw_rect_dsc_init(&mid_dsc);
        lv_obj_init_draw_rect_dsc(obj, LV_PART_INDICATOR, &mid_dsc);
        lv_coord_t w = lv_obj_get_style_width(obj, LV_PART_INDICATOR) / 2;
        lv_coord_t h = lv_obj_get_style_height(obj, LV_PART_INDICATOR) / 2;
        lv_area_t nm_cord;
        nm_cord.x1 = scale_center.x - w;
        nm_cord.y1 = scale_center.y - h;
        nm_cord.x2 = scale_center.x + w;
        nm_cord.y2 = scale_center.y + h;
        lv_draw_rect(layer, &mid_dsc, &nm_cord);
    }
}

static void draw_arcs(lv_obj_t * obj, lv_layer_t * layer, const lv_area_t * scale_area)
{
    lv_meter_t * meter = (lv_meter_t *)obj;

    lv_draw_arc_dsc_t arc_dsc;
    lv_draw_arc_dsc_init(&arc_dsc);
    arc_dsc.rounded = lv_obj_get_style_arc_rounded(obj, LV_PART_ITEMS);

    lv_coord_t r_out = lv_area_get_width(scale_area) / 2 ;
    lv_point_t scale_center;
    scale_center.x = scale_area->x1 + r_out;
    scale_center.y = scale_area->y1 + r_out;

    lv_opa_t opa_main = lv_obj_get_style_opa_recursive(obj, LV_PART_MAIN);
    lv_meter_indicator_t * indic;

    _LV_LL_READ_BACK(&meter->indicator_ll, indic) {
        if(indic->type != LV_METER_INDICATOR_TYPE_ARC) continue;

        arc_dsc.color = indic->type_data.arc.color;
        arc_dsc.img_src = indic->type_data.arc.src;
        arc_dsc.width = indic->type_data.arc.width;
        arc_dsc.opa = indic->opa > LV_OPA_MAX ? opa_main : (opa_main * indic->opa) >> 8;

        int32_t start_angle = lv_map(indic->start_value, meter->scale.min, meter->scale.max, meter->scale.rotation,
                                     meter->scale.rotation + meter->scale.angle_range);
        int32_t end_angle = lv_map(indic->end_value, meter->scale.min, meter->scale.max, meter->scale.rotation,
                                   meter->scale.rotation + meter->scale.angle_range);

        lv_coord_t radius = r_out + indic->type_data.arc.r_mod;

        arc_dsc.center = scale_center;
        arc_dsc.radius = radius;
        arc_dsc.start_angle = start_angle;
        arc_dsc.end_angle = end_angle;
        lv_draw_arc(layer, &arc_dsc);
    }
}

static void draw_ticks_and_labels(lv_obj_t * obj, lv_layer_t * layer, const lv_area_t * scale_area)
{
    lv_meter_t * meter    = (lv_meter_t *)obj;

    lv_point_t p_center;
    lv_coord_t r_edge = LV_MIN(lv_area_get_width(scale_area) / 2, lv_area_get_height(scale_area) / 2);
    p_center.x = scale_area->x1 + r_edge;
    p_center.y = scale_area->y1 + r_edge;

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_TICKS, &line_dsc);

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    lv_obj_init_draw_label_dsc(obj, LV_PART_TICKS, &label_dsc);

    lv_coord_t r_out = r_edge;
    lv_coord_t r_in_major = r_out - meter->scale.tick_major_length;
    lv_coord_t r_in_minor = r_out - meter->scale.tick_length;

    uint32_t minor_cnt = meter->scale.tick_major_nth ? meter->scale.tick_major_nth - 1 : 0xFFFF;
    uint16_t i;
    for(i = 0; i < meter->scale.tick_cnt; i++) {
        minor_cnt++;
        bool major = false;
        if(minor_cnt == meter->scale.tick_major_nth) {
            minor_cnt = 0;
            major = true;
        }

        int32_t value_of_line = lv_map(i, 0, meter->scale.tick_cnt - 1, meter->scale.min, meter->scale.max);
        lv_color_t line_color = major ? meter->scale.tick_major_color : meter->scale.tick_color;
        lv_coord_t line_width = major ? meter->scale.tick_major_width : meter->scale.tick_width;

        lv_meter_indicator_t * indic;
        _LV_LL_READ_BACK(&meter->indicator_ll, indic) {
            if(indic->type != LV_METER_INDICATOR_TYPE_SCALE_LINES) continue;
            if(value_of_line >= indic->start_value && value_of_line <= indic->end_value) {
                line_width += indic->type_data.scale_lines.width_mod;

                if(lv_color_eq(indic->type_data.scale_lines.color_start, indic->type_data.scale_lines.color_end)) {
                    line_color = indic->type_data.scale_lines.color_start;
                }
                else {
                    lv_opa_t ratio;
                    if(indic->type_data.scale_lines.local_grad) {
                        ratio = lv_map(value_of_line, indic->start_value, indic->end_value, LV_OPA_TRANSP, LV_OPA_COVER);
                    }
                    else {
                        ratio = lv_map(value_of_line, meter->scale.min, meter->scale.max, LV_OPA_TRANSP, LV_OPA_COVER);
                    }
                    line_color = lv_color_mix(indic->type_data.scale_lines.color_end, indic->type_data.scale_lines.color_start, ratio);
                }
            }
        }

        int32_t angle_upscale = ((i * meter->scale.angle_range) * 10) / (meter->scale.tick_cnt - 1);
        angle_upscale += meter->scale.rotation * 10;

        line_dsc.color = line_color;
        line_dsc.width = line_width;

        /*Draw a little bit longer lines to be sure the mask will clip them correctly
         *and to get a better precision*/
        lv_point_t p_outer;
        p_outer.x = p_center.x + r_out;
        p_outer.y = p_center.y;
        lv_point_transform(&p_outer, angle_upscale, 256, &p_center);

        lv_point_t p_inner;
        p_inner.x = p_center.x + (major ? r_in_major : r_in_minor);
        p_inner.y = p_center.y;
        lv_point_transform(&p_inner, angle_upscale, 256, &p_center);

        /*Draw the text*/
        if(major) {
            uint32_t r_text = r_in_major - meter->scale.label_gap;
            lv_point_t p;
            p.x = p_center.x + r_text;
            p.y = p_center.y;
            lv_point_transform(&p, angle_upscale, 256, &p_center);

            lv_draw_label_dsc_t label_dsc_tmp;
            lv_memcpy(&label_dsc_tmp, &label_dsc, sizeof(label_dsc_tmp));

            char buf[16];

            lv_snprintf(buf, sizeof(buf), "%" LV_PRId32, value_of_line);

            lv_point_t label_size;
            label_dsc_tmp.text = buf;
            lv_text_get_size(&label_size, buf, label_dsc.font, label_dsc.letter_space, label_dsc.line_space,
                             LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t label_cord;
            label_cord.x1 = p.x - label_size.x / 2;
            label_cord.y1 = p.y - label_size.y / 2;
            label_cord.x2 = label_cord.x1 + label_size.x;
            label_cord.y2 = label_cord.y1 + label_size.y;

            lv_draw_label(layer, &label_dsc, &label_cord);
        }

        line_dsc.p1 = p_outer;
        line_dsc.p2 = p_inner;
        lv_draw_line(layer, &line_dsc);
    }
}


static void draw_needles(lv_obj_t * obj, lv_layer_t * layer, const lv_area_t * scale_area)
{
    lv_meter_t * meter = (lv_meter_t *)obj;

    lv_coord_t r_edge = lv_area_get_width(scale_area) / 2;
    lv_point_t scale_center;
    scale_center.x = scale_area->x1 + r_edge;
    scale_center.y = scale_area->y1 + r_edge;

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_ITEMS, &line_dsc);

    lv_draw_image_dsc_t img_dsc;
    lv_draw_image_dsc_init(&img_dsc);
    lv_obj_init_draw_image_dsc(obj, LV_PART_ITEMS, &img_dsc);
    lv_opa_t opa_main = lv_obj_get_style_opa_recursive(obj, LV_PART_MAIN);

    lv_meter_indicator_t * indic;
    _LV_LL_READ_BACK(&meter->indicator_ll, indic) {

        if(indic->type == LV_METER_INDICATOR_TYPE_NEEDLE_LINE) {
            int32_t angle = lv_map(indic->end_value, meter->scale.min, meter->scale.max, meter->scale.rotation,
                                   meter->scale.rotation + meter->scale.angle_range);
            lv_coord_t r_out = r_edge + meter->scale.r_mod + indic->type_data.needle_line.r_mod;
            lv_point_t p_end;
            p_end.y = (lv_trigo_sin(angle) * (r_out)) / LV_TRIGO_SIN_MAX + scale_center.y;
            p_end.x = (lv_trigo_cos(angle) * (r_out)) / LV_TRIGO_SIN_MAX + scale_center.x;
            line_dsc.color = indic->type_data.needle_line.color;
            line_dsc.width = indic->type_data.needle_line.width;
            line_dsc.opa = indic->opa > LV_OPA_MAX ? opa_main : (opa_main * indic->opa) >> 8;
            line_dsc.p1 = scale_center;
            line_dsc.p2 = p_end;
            lv_draw_line(layer, &line_dsc);
        }
        else if(indic->type == LV_METER_INDICATOR_TYPE_NEEDLE_IMG) {
            if(indic->type_data.needle_img.src == NULL) continue;

            int32_t angle = lv_map(indic->end_value, meter->scale.min, meter->scale.max, meter->scale.rotation,
                                   meter->scale.rotation + meter->scale.angle_range);
            lv_image_header_t info;
            lv_image_decoder_get_info(indic->type_data.needle_img.src, &info);
            lv_area_t a;
            a.x1 = scale_center.x - indic->type_data.needle_img.pivot.x;
            a.y1 = scale_center.y - indic->type_data.needle_img.pivot.y;
            a.x2 = a.x1 + info.w - 1;
            a.y2 = a.y1 + info.h - 1;

            img_dsc.opa = indic->opa > LV_OPA_MAX ? opa_main : (opa_main * indic->opa) >> 8;
            img_dsc.pivot.x = indic->type_data.needle_img.pivot.x;
            img_dsc.pivot.y = indic->type_data.needle_img.pivot.y;
            angle = angle * 10;
            if(angle > 3600) angle -= 3600;
            img_dsc.rotation = angle;

            img_dsc.src = indic->type_data.needle_img.src;
            lv_draw_image(layer, &img_dsc, &a);
        }
    }
}

static void inv_arc(lv_obj_t * obj, lv_meter_indicator_t * indic, int32_t old_value, int32_t new_value)
{
    lv_meter_t * meter = (lv_meter_t *)obj;

    bool rounded = lv_obj_get_style_arc_rounded(obj, LV_PART_ITEMS);

    lv_area_t scale_area;
    lv_obj_get_content_coords(obj, &scale_area);

    lv_coord_t r_out = lv_area_get_width(&scale_area) / 2;
    lv_point_t scale_center;
    scale_center.x = scale_area.x1 + r_out;
    scale_center.y = scale_area.y1 + r_out;

    r_out += indic->type_data.arc.r_mod;

    int32_t start_angle = lv_map(old_value, meter->scale.min, meter->scale.max, meter->scale.rotation,
                                 meter->scale.angle_range + meter->scale.rotation);
    int32_t end_angle = lv_map(new_value, meter->scale.min, meter->scale.max, meter->scale.rotation,
                               meter->scale.angle_range + meter->scale.rotation);

    lv_area_t a;
    lv_draw_arc_get_area(scale_center.x, scale_center.y, r_out, LV_MIN(start_angle, end_angle), LV_MAX(start_angle,
                                                                                                       end_angle), indic->type_data.arc.width, rounded, &a);
    lv_obj_invalidate_area(obj, &a);
}


static void inv_line(lv_obj_t * obj, lv_meter_indicator_t * indic, int32_t value)
{
    lv_meter_t * meter = (lv_meter_t *)obj;

    lv_area_t scale_area;
    lv_obj_get_content_coords(obj, &scale_area);

    lv_coord_t r_out = lv_area_get_width(&scale_area) / 2;
    lv_point_t scale_center;
    scale_center.x = scale_area.x1 + r_out;
    scale_center.y = scale_area.y1 + r_out;

    if(indic->type == LV_METER_INDICATOR_TYPE_NEEDLE_LINE) {
        int32_t angle = lv_map(value, meter->scale.min, meter->scale.max, meter->scale.rotation,
                               meter->scale.rotation + meter->scale.angle_range);
        r_out += meter->scale.r_mod + indic->type_data.needle_line.r_mod;
        lv_point_t p_end;
        p_end.y = (lv_trigo_sin(angle) * (r_out)) / LV_TRIGO_SIN_MAX + scale_center.y;
        p_end.x = (lv_trigo_cos(angle) * (r_out)) / LV_TRIGO_SIN_MAX + scale_center.x;

        lv_area_t a;
        a.x1 = LV_MIN(scale_center.x, p_end.x) - indic->type_data.needle_line.width - 2;
        a.y1 = LV_MIN(scale_center.y, p_end.y) - indic->type_data.needle_line.width - 2;
        a.x2 = LV_MAX(scale_center.x, p_end.x) + indic->type_data.needle_line.width + 2;
        a.y2 = LV_MAX(scale_center.y, p_end.y) + indic->type_data.needle_line.width + 2;

        lv_obj_invalidate_area(obj, &a);
    }
    else if(indic->type == LV_METER_INDICATOR_TYPE_NEEDLE_IMG) {
        int32_t angle = lv_map(value, meter->scale.min, meter->scale.max, meter->scale.rotation,
                               meter->scale.rotation + meter->scale.angle_range);
        lv_image_header_t info;
        lv_image_decoder_get_info(indic->type_data.needle_img.src, &info);

        angle = angle * 10;
        if(angle > 3600) angle -= 3600;

        scale_center.x -= indic->type_data.needle_img.pivot.x;
        scale_center.y -= indic->type_data.needle_img.pivot.y;
        lv_area_t a;
        _lv_image_buf_get_transformed_area(&a, info.w, info.h, angle, LV_SCALE_NONE, &indic->type_data.needle_img.pivot);
        a.x1 += scale_center.x - 2;
        a.y1 += scale_center.y - 2;
        a.x2 += scale_center.x + 2;
        a.y2 += scale_center.y + 2;

        lv_obj_invalidate_area(obj, &a);
    }
}
#endif
