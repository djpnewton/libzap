package com.djpsoft.zap.plugin;

public final class IntResult {

    public final boolean Success;
    public final long Value;

    public IntResult(boolean success, long value) {
        this.Success = success;
        this.Value = value;
    }

    @Override
    public String toString() {
        return "IntResult{" +
                "Success=" + Success +
                ", Value=" + Value +
        "}";
    }
}
