package com.djpsoft.zap.plugin;

public final class IntResult {

    final boolean Success;
    final long Value;

    public IntResult(boolean success, int value) {
        this.Success = success;
        this.Value = value;
    }

    public boolean getSuccess() {
        return Success;
    }

    public int getValue() {
        return Value;
    }

    @Override
    public String toString() {
        return "IntResult{" +
                "Success=" + Success +
                ", Value=" + Value +
        "}";
    }
}
