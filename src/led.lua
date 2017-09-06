local led = {
    RED = 0,
    GREEN = 1,
    YELLOW =2,
};

local tmr_red, tmr_green, tmr_yellow

function led.init()
    gpio.mode(led.RED, gpio.OUTPUT, gpio.PULLUP)
    gpio.mode(led.GREEN, gpio.OUTPUT)
    gpio.mode(led.GREEN, gpio.OUTPUT)
end

function led.on(pin)
    tmr.unregister(pin)
    gpio.write(pin, gpio.HIGH)
end

function led.off(pin)
    tmr.unregister(pin)
    gpio.write(pin, gpio.LOW)
end

function led.click(pin, speed)
    speed = speed or 300
    gpio.write(pin, gpio.LOW)
    tmr.alarm(pin, speed, tmr.ALARM_SINGLE, function ()
        gpio.write(pin, gpio.HIGH)
    end)
end

function led.blick(pin, speed)
    local state = true
    speed = speed or 200
    gpio.write(pin, gpio.HIGH)
    tmr.alarm(pin, speed, tmr.ALARM_AUTO, function ()
        gpio.write(pin, state and gpio.HIGH or gpio.LOW)
        state = not state
    end)
end

return led
