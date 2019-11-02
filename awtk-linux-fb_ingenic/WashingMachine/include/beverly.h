#ifndef __BEVERLY_H
#define __BEVERLY_H

#define EVT_BACK_TO_HOME             0x2001
#define EVT_WASH_STEP_TIME_CHANGED   0x2002

enum WASHINGSTEP {
	STEP_WASH,
	STEP_RINSE,
	STEP_DEWATER
};

typedef struct _WashingStepValue {
	int type;
	int result;
}WashingStepValue;

static const char* sWashTime[18] = {
	"",
	"88",
	"91",
	"15",
	"95",
	"68",
	"45",
	"65",
	"50",
	"90",
	"80",
	"60",
	"70",
	"30",
	"120",
	"30",
	"60",
	"78"
};

static const char* sFunction[18] = {
	"智能洗衣专家",
	"棉麻",
	"混合洗",
	"快速洗15'",
	"大件",
	"羊毛",
	"节能",
	"活性酶",
	"空气洗",
	"羽绒服",
	"运动服",
	"户外服",
	"漂洗+脱水",
	"单脱水",
	"单烘干",
	"速洗30'",
	"洗洪60'",
	"筒自洁"
};

static const char* sFuncDescription[18] = {
	"全智能感知",
	"日常棉麻衣物",
	"混合衣物清洗",
	"微脏的少件衣物",
	"针对大件衣物清洗",
	"羊毛类衣物清洗",
	"节能清洗",
	"活性酶消毒杀菌",
	"暖风清新衣物",
	"羽绒类衣物清洗",
	"运动服衣物清洗",
	"户外服衣物清洗",
	"漂洗脱水一步到位",
	"单独的脱水程序",
	"单独的烘干程序",
	"快速洗净衣物",
	"快洗烘干一步到位",
	"特设的自清洁程序"
};

static const char* sFuncStepWash[18] = {
	"",
	"18|分|洗涤",
	"20|分|洗涤",
	"10|分|洗涤",
	"30|分|洗涤",
	"15|分|洗涤",
	"10|分|洗涤",
	"20|分|洗涤",
	"",
	"20|分|洗涤",
	"15|分|洗涤",
	"20|分|洗涤",
	"18|分|洗涤",
	"-|L|水位",
	"-|分|洗涤",
	"10|分|洗涤",
	"30|分|洗涤",
	"-|分|洗涤",
};

static const char* sFuncStepRinse[18] = {
	"",
	"2|次|漂洗",
	"3|次|漂洗",
	"2|次|漂洗",
	"4|次|漂洗",
	"3|次|漂洗",
	"2|次|漂洗",
	"2|次|漂洗",
	"",
	"2|次|漂洗",
	"2|次|漂洗",
	"3|次|漂洗",
	"5|次|漂洗",
	"-|度|温度",
	"-|次|漂洗",
	"2|次|漂洗",
	"3|次|漂洗",
	"1|次|漂洗",
};

static const char* sFuncStepDewater[18] = {
	"",
	"7|分|脱水",
	"10|分|脱水",
	"5|分|脱水",
	"15|分|脱水",
	"10|分|脱水",
	"5|分|脱水",
	"15|分|脱水",
	"",
	"10|分|脱水",
	"9|分|脱水",
	"12|分|脱水",
	"20|分|脱水",
	"30|分|脱水",
	"50|分|烘干",
	"5|分|脱水",
	"25|分|烘干",
	"-|分|脱水",
};

ret_t start_settings_activity();
ret_t start_washing_mode_activity(int index);
ret_t start_wash_step_activity(int step);
ret_t start_washing_function_menu_activity();
ret_t start_washing_step_activity(int step, int result);
ret_t show_star_alert_dialog(bool_t favorite);

#endif /*__BEVERLY_H*/
