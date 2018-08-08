package com.djpsoft.zap.plugin;

public final class SpendTx {

    public boolean Success;
    public byte[] TxData;
    public byte[] Signature;

    public SpendTx(boolean success, byte[] txData, byte[] signature) {
        this.Success = Success;
        this.TxData = txData;
        this.Signature = Signature;
    }

    @Override
    public String toString() {
        return "SpendTx{}";
    }
}
