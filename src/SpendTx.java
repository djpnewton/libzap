package com.djpsoft.zap.plugin;

public final class SpendTx {

    public boolean Success;
    public byte[] TxData;
    public byte[] Signature;

    public SpendTx(boolean success, byte[] txData, byte[] signature) {
        this.Success = success;
        this.TxData = txData;
        this.Signature = signature;
    }

    @Override
    public String toString() {
        return "SpendTx{}";
    }
}
