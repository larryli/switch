local btn = {
    PIN = 5,
}

local timer

function btn.init()
    gpio.mode(btn.PIN, gpio.INT)
    timer = tmr.create()
end

function btn.push(callback)
    gpio.trig(btn.PIN, "down", callback)
end

function btn.off()
    gpio.trig(btn.PIN)
end

function btn.hold(callback, timed)
    timed = timed or 1500
    gpio.trig(btn.PIN, "both", function(level, when)
        if level == 1 then
            tmr.alarm(timer, timed, tmr.ALARM_SINGLE, function ()
                callback()
            end)
        else
            tmr.unregister(timer)
        end
    end)
end

return btn
