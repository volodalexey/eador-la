package com.eador.reforged;

import android.app.NativeActivity;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.WindowManager;

public class GameActivity extends NativeActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // NativeActivity ставит ADJUST_RESIZE; экранная клавиатура не должна менять окно игры.
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            int unicode = event.getUnicodeChar(event.getMetaState());
            if (unicode > 0) {
                nativeOnChar(unicode);
            }
        } else if (event.getAction() == KeyEvent.ACTION_MULTIPLE) {
            String chars = event.getCharacters();
            if (chars != null) {
                for (int i = 0; i < chars.length(); ) {
                    int codePoint = chars.codePointAt(i);
                    nativeOnChar(codePoint);
                    i += Character.charCount(codePoint);
                }
            }
        }
        return super.dispatchKeyEvent(event);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        System.exit(0);
    }

    private static native void nativeOnChar(int codePoint);
}
