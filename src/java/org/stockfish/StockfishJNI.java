package org.stockfish;

public final class StockfishJNI implements AutoCloseable {
    static {
        System.loadLibrary("jstockfish");
    }

    private long handle;

    public StockfishJNI() {
        handle = nativeCreate();
    }

    public void setPosition(String fen, String... moves) {
        nativeSetPosition(handle, fen, moves);
    }

    public boolean makeMove(String move) {
        return nativeMakeMove(handle, move);
    }

    public int staticEvalCp() {
        return nativeStaticEvalCp(handle);
    }

    public String bestMoveAtDepth(int depth) {
        return nativeBestMoveDepth(handle, depth);
    }

    @Override
    public void close() {
        if (handle != 0L) {
            nativeDestroy(handle);
            handle = 0L;
        }
    }

    private native long nativeCreate();
    private native void nativeDestroy(long handle);
    private native void nativeSetPosition(long handle, String fen, String[] moves);
    private native boolean nativeMakeMove(long handle, String move);
    private native int nativeStaticEvalCp(long handle);
    private native String nativeBestMoveDepth(long handle, int depth);
}
