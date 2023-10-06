# ПРОТОКОЛ ИЗМЕРЕНИЙ

![](data:image/png;base64,{{first(imgs)}}){ width=100% }


{% for i in range(length(traces)) %}
### {{at(traces, i).name}}

![](data:image/png;base64,{{at(imgs,i+1)}}){ width=100% }

Параметр|Значение
-----|-----:{%for p in at(traces, i).params %}
{{p.name}}|{{p.value}}{% endfor %}

{%endfor%}
