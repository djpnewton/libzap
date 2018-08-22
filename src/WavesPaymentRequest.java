package com.djpsoft.zap.plugin;

public final class WavesPaymentRequest {

    public String Address;
    public String AssetId;
    public String Attachment;
    public long Amount;

    public WavesPaymentRequest() {
    }

    @Override
    public String toString() {
        return "WavesPaymentRequest{" +
                "Address=" + Address +
                ", AssetId=" + AssetId +
                ", Attachment=" + Attachment +
                ", Amount=" + Amount +
        "}";
    }
}
