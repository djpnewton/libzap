package com.djpsoft.zap.plugin;

public class zap_jni {
    private static final String TAG = "libzap";

    /* this is used to load the 'hello-jni' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.djpsoft.zap.plugin/lib/libzap.so at
     * installation time by the package manager.
     */
    static {
        System.loadLibrary("zap");
    }

    /* A native method that is implemented by the
     * 'zap' native library, which is packaged
     * with this application.
     */
    public static native int error(String[] msg_out);
    public static native int version();
    public static native String node_get();
    public static native void node_set(String url);
    public static native char network_get();
    public static native int network_set(char network_byte);
    public static native String asset_id_get();
    public static native void asset_id_set(String asset_id);
    public static native String mnemonic_create();
    public static native int mnemonic_check(String mnemonic);
    public static native String[] mnemonic_wordlist();
    public static native String seed_address(String seed);
    public static native IntResult address_check(String address);
    public static native IntResult address_balance(String address);
    public static native IntResult address_transactions(String address, Tx[] txs, int count, String after);
    public static native IntResult transaction_fee();
    public static native SpendTx transaction_create(String seed, String recipient, long amount, long fee, String attachment);
    public static native int transaction_broadcast(SpendTx spend_tx, Tx broadcast_tx);
    public static native int uri_parse(String uri, WavesPaymentRequest req);
}
