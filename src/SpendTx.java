package com.djpsoft.zap.plugin;

public final class SpendTx {

    public boolean Success;
    public byte[] Data;
    public byte[] Signature;

    public SpendTx(boolean success, byte[] Data, byte[] signature) {
        this.Success = success;
        this.Data = Data;
        this.Signature = signature;
    }

    @Override
    public String toString() {
        return "SpendTx{}";
    }
}
