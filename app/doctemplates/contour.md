
# ПРОТОКОЛ ИЗМЕРЕНИЙ

Деталь: {{part_name}} 
Дата: {{date}}

![](data:image/png;base64,{{first(imgs)}}){ width=100% }

{% if length(default(distances,[])) > 0 %}
### Расстояния
Измерение|Значение
-----|-----:
{% for g in distances %}**{{g.name}}**|{{g.mean}}
{% for m in g.measurements %}{{m.name}}|{{m.value}}
{%endfor%}{%endfor%}
{% endif %}

{% if length(default(angles,[])) > 0 %}
### Углы

Измерение|Значение
-----|-----:
{% for g in angles %}**{{g.name}}**|{{g.mean}}
{% for m in g.measurements %}{{m.name}}|{{m.value}}
{%endfor%}{%endfor%}
{% endif %}

{% if length(default(rads,[])) > 0 %}
### Радиусы

Измерение|Значение
-----|-----:
{% for g in rads %}**{{g.name}}**|{{g.mean}}
{% for m in g.measurements %}{{m.name}}|{{m.value}}
{%endfor%}{%endfor%}
{% endif %}




