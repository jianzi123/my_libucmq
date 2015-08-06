参数                     名称                                 注释

trigger                触发类型                          一共4种，任选一种
every_time             随时触发
power_on               开机触发
one_day                每天触发                          触发次数可选
fixed_time             固定间隔触发                      触发次数可选，触发间隔可选
fix_num                触发次数                          整数
deloy                  触发间隔                          单位：秒
service_type           服务类型                          长度为6
interval               weibo有效时长                     单位：秒
level                  weibo等级                         0～99 数子越小优先级越高
yes                    YES键                             0为不允许，1为允许
text                   weibo文本                         不超过80字
position_type          数据类型                          从数据库daokemap中查找
pos_type               道路类型                          选择一种 
highway                高度公路
urban                  城区
speed                  速度
time_type              统计类型                          有2中可选：单次和总数 
mileage                里程                              单位米
time                   时间                              单位：秒
fun_type               函数类型                          选择一种
get_grid_content       获取格网中文本              
get_off_site_content   获取异地行政文本
time_start             开始时间(s)(小时 * 60 * 60 )      逻辑判断的开始时间  在10 ~ 14点之间，也可以设置为跨天 23 ~ 04 点
time_end               结束时间(s)(小时 * 60 * 60 )      逻辑判断的结束时间  在10 ~ 14点之间，也可以设置为跨天 23 ~ 04 点
min_time_unit          初始化的最小间隔时间              只有adtalk模板才使用
up_line                每次初始化的广告上限              只有adtalk模板才使用 
