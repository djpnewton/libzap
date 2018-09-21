package com.djpsoft.zap.plugin;

public final class Tx {

    public long Type;
    public String Id;
    public String Sender;
    public String Recipient;
    public String AssetId;
    public String FeeAsset;
    public String Attachment;
    public long Amount;
    public long Fee;
    public long Timestamp;

    public Tx() {
    }

    @Override
    public String toString() {
        return "Tx{" +
                "Type=" + Type +
                "  Sender=" + Sender +
                ", Recipient=" + Recipient +
                ", Amount=" + Amount +
                ", AssetId=" + AssetId +
        "}";
    }
}
