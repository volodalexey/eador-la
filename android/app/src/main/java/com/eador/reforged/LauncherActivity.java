package com.eador.reforged;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.DocumentsContract;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class LauncherActivity extends Activity {
    private static final String TAG = "EadorLauncher";
    private static final int REQUEST_EXE = 1;
    private static final int REQUEST_FOLDER = 2;

    private TextView statusView;
    private ExecutorService executor;
    private Handler mainHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        File dataDir = getExternalFilesDir(null);
        if (dataDir == null) {
            finish();
            return;
        }

        // Проверка наличия игровых данных
        if (hasGameData(dataDir)) {
            startGame();
            return;
        }

        // UI для импорта
        setContentView(R.layout.activity_launcher);

        Button btnExe = findViewById(R.id.btn_import_exe);
        Button btnFolder = findViewById(R.id.btn_import_folder);
        statusView = findViewById(R.id.status);

        executor = Executors.newSingleThreadExecutor();
        mainHandler = new Handler(Looper.getMainLooper());

        btnExe.setOnClickListener(v -> {
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
            intent.setType("*/*");
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            startActivityForResult(intent, REQUEST_EXE);
        });

        btnFolder.setOnClickListener(v -> {
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
            startActivityForResult(intent, REQUEST_FOLDER);
        });
    }

    private boolean hasGameData(File dataDir) {
        return new File(dataDir, "dat/Interface.dat").exists() &&
               new File(dataDir, "var").isDirectory() &&
               (new File(dataDir, "text").isDirectory() || new File(dataDir, "Text").isDirectory()) &&
               new File(dataDir, "font01.txt").exists();
    }

    private void startGame() {
        Intent intent = new Intent(this, GameActivity.class);
        startActivity(intent);
        finish();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode != RESULT_OK || data == null) {
            return;
        }

        Uri uri = data.getData();
        if (uri == null) {
            return;
        }

        File dataDir = getExternalFilesDir(null);
        if (dataDir == null) {
            return;
        }

        if (requestCode == REQUEST_EXE) {
            handleExeImport(uri, dataDir);
        } else if (requestCode == REQUEST_FOLDER) {
            handleFolderImport(uri, dataDir);
        }
    }

    private void handleExeImport(Uri uri, File dataDir) {
        executor.execute(() -> {
            try {
                // Проверка свободного места (~600 МБ)
                long freeSpace = dataDir.getUsableSpace();
                long needed = 600L * 1024 * 1024;
                if (freeSpace < needed) {
                    String msg = getString(R.string.not_enough_space, 600);
                    mainHandler.post(() -> statusView.setText(msg));
                    return;
                }

                System.loadLibrary("eador");

                // Копирование в cache
                mainHandler.post(() -> statusView.setText(R.string.copying));
                File cacheExe = new File(getCacheDir(), "setup.exe");
                try (InputStream in = getContentResolver().openInputStream(uri);
                     FileOutputStream out = new FileOutputStream(cacheExe)) {
                    byte[] buf = new byte[8192];
                    int len;
                    while ((len = in.read(buf)) > 0) {
                        out.write(buf, 0, len);
                    }
                }

                // Импорт через JNI
                String result = nativeImportExe(cacheExe.getAbsolutePath(), dataDir.getAbsolutePath());
                cacheExe.delete();

                if (result == null) {
                    mainHandler.post(this::startGame);
                } else {
                    String msg = getString(R.string.import_failed, result);
                    mainHandler.post(() -> statusView.setText(msg));
                }
            } catch (Exception e) {
                Log.e(TAG, "Import failed", e);
                String msg = getString(R.string.import_failed, e.getMessage());
                mainHandler.post(() -> statusView.setText(msg));
            }
        });
    }

    private void handleFolderImport(Uri treeUri, File dataDir) {
        executor.execute(() -> {
            try {
                System.loadLibrary("eador");

                File cacheDir = new File(getCacheDir(), "gog_game");
                cacheDir.mkdirs();

                mainHandler.post(() -> statusView.setText(R.string.copying));
                copyTree(treeUri, DocumentsContract.getTreeDocumentId(treeUri), cacheDir, true);

                String result = nativeImportFolder(cacheDir.getAbsolutePath(), dataDir.getAbsolutePath());
                deleteRecursive(cacheDir);

                if (result == null) {
                    mainHandler.post(this::startGame);
                } else {
                    String msg = getString(R.string.import_failed, result);
                    mainHandler.post(() -> statusView.setText(msg));
                }
            } catch (Exception e) {
                Log.e(TAG, "Import failed", e);
                String msg = getString(R.string.import_failed, e.getMessage());
                mainHandler.post(() -> statusView.setText(msg));
            }
        });
    }

    // В корне копируются только нужные элементы, внутри каталогов — всё.
    private static boolean isWantedRootEntry(String name) {
        String n = name.toLowerCase(java.util.Locale.ROOT);
        return n.equals("dat") || n.equals("text") || n.equals("var") ||
               n.matches("font0[12]\\.(pcx|txt)") ||
               (n.startsWith("goggame-") && n.endsWith(".ico"));
    }

    private void copyTree(Uri treeUri, String parentDocId, File destDir, boolean root) throws Exception {
        Uri childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, parentDocId);

        try (Cursor cursor = getContentResolver().query(childrenUri,
                new String[]{DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                             DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                             DocumentsContract.Document.COLUMN_MIME_TYPE},
                null, null, null)) {

            if (cursor == null) return;

            while (cursor.moveToNext()) {
                String docId = cursor.getString(0);
                String name = cursor.getString(1);
                String mimeType = cursor.getString(2);

                if (root && !isWantedRootEntry(name)) continue;

                Uri docUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, docId);

                if (DocumentsContract.Document.MIME_TYPE_DIR.equals(mimeType)) {
                    File subDir = new File(destDir, name);
                    subDir.mkdirs();
                    copyTree(treeUri, docId, subDir, false);
                } else {
                    File destFile = new File(destDir, name);
                    try (InputStream in = getContentResolver().openInputStream(docUri);
                         FileOutputStream out = new FileOutputStream(destFile)) {
                        byte[] buf = new byte[8192];
                        int len;
                        while ((len = in.read(buf)) > 0) {
                            out.write(buf, 0, len);
                        }
                    }
                }
            }
        }
    }

    private void deleteRecursive(File file) {
        if (file.isDirectory()) {
            File[] children = file.listFiles();
            if (children != null) {
                for (File child : children) {
                    deleteRecursive(child);
                }
            }
        }
        file.delete();
    }

    public void onProgress(long done, long total) {
        int percent = (int)(done * 100 / Math.max(total, 1));
        String msg = getString(R.string.importing, percent);
        mainHandler.post(() -> statusView.setText(msg));
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (executor != null) {
            executor.shutdownNow();
        }
    }

    private native String nativeImportExe(String exePath, String dataDir);
    private native String nativeImportFolder(String sourceDir, String dataDir);
}
