// 提交开关操作
// 压缩代码 http://www.minifier.org/
var turn = (D => { // D：处理调试信息的回调，默认为空将使用 console.log 输出
    var _ = !!D ? D : (...a) => console.log.apply(console, a), // console.log
        I = s => document.getElementById(s), // 指定 ID 的元素
        T = (s, b) => I(s).className = b ? '' : 'hidden', // 切换指定 ID 元素是否显示
        S = v => document.getElementsByName('switches').forEach(o => o.value = v), // 切换提交表单返回是否包含 switches
        R = s => { // 返回数据处理
            var n = 0, // 是否显示全开
                f = 0, // 是否显示全关
                o = JSON.parse(s);
            _(o, s);
            !!o.success && 1 == o.success && !!o.switches && ( // 返回数据成功且包含 switches 时才处理
                o.switches.forEach( // 遍历 switches 数据，处理每个开关
                    i => {
                        var t = I('switch-' + i.switch), // 指定开关的表单
                            b = 1 == i.state;
                        t.querySelector('input[name=state]').value = b ? 0 : 1; // 切换值
                        t.querySelector('span').className = b ? 'on' : ''; // 切换显示
                        n = n || !b;
                        f = f || b
                    }
                ), T('switch-on', n), T('switch-off', f) // 处理全开、全关显示
            )
        },
        E; // SSE 对象
    !!window.EventSource && ( // 支持 SSE
        (E = new EventSource('/events')).onopen = e => _(e) || S(0), // 连接，成功后禁止提交表单返回 switches
        E.onerror = e => _(e) || S(e.target.readyState == EventSource.OPEN ? 0 : 1), // 断线与重连处理
        E.onmessage = e => _(e) || R(e.data) // 接收消息处理开关
    );

    return (o, p) => { // 触发
        var f = o.parentNode,
            d = new FormData,
            r = new XMLHttpRequest;

        !!p && (f = f.parentNode); // 开关表单比全开、全关要多包一层 <label/>
        Array.from(f.children).forEach( // 遍历表单，构造数据
            o => !!o.name && !!o.value && d.append(o.name, o.value)
        );
        r.onreadystatechange = ( // 返回数据处理
            () => 4 == r.readyState && 200 == r.status && R(r.responseText)
        );
        r.open(f.method, f.action);
        _(f, d, r);
        r.send(d);
        return !1
    }
})(() => {}); // 禁止调试输出
