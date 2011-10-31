//----------------------------------------------------------------------------------
// File:            libs\src\com\nvidia\devtech\NvAPKFileHelper.java
// Samples Version: Android NVIDIA samples 2 
// Email:           tegradev@nvidia.com
// Forum:           http://developer.nvidia.com/tegra/forums/tegra-forums/android-development
//
// Copyright 2009-2010 NVIDIA® Corporation 
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//----------------------------------------------------------------------------------

package com.nvidia.devtech;

import java.io.IOException;
import java.io.InputStream;

import android.content.Context;

/**
 * A helper class used to aid native code reading files from the assets
 * directory in an APK. Normally you would not use this code in Java, but rather
 * use the NvAPK* functions in your native code by including
 * <nv_apk_file/nv_apk_file.h>
 */
public class NvAPKFileHelper
{
    private Context context = null;
    private static NvAPKFileHelper instance = new NvAPKFileHelper();

    public void setContext(Context context)
    {
        this.context = context;
    }

    public static NvAPKFileHelper getInstance()
    {
        return instance;
    }

    public class NvAPKFile
    {
        /** The actual data bytes. */
        public byte[] data;
        public int bufferSize;
        /** The length of the data. */
        public int length;
        /** The length of the data. */
        public int position;
        /** Stream used to push/pull data. */
        public InputStream is;
    }

    public NvAPKFile openFileAndroid(String filename)
    {
        NvAPKFile ret = new NvAPKFile();
        ret.is = null;
        ret.length = 0;
        ret.position = 0;
        ret.bufferSize = 0;

        try
        {
            ret.is = context.getAssets().open(filename);
            int size = ret.is.available();
            ret.length = size;
            ret.is.mark(1024 * 1024 * 256);
            ret.bufferSize = 1024;
            ret.data = new byte[ret.bufferSize];
        }
        catch (Exception e2)
        {
            System.out.println("openFileAndroid \"" + filename
                    + "\" not found in assets");
            ret = null;
        }
        return ret;
    }

    public void readFileAndroid(NvAPKFile strm, int size)
    {
        if (size > strm.bufferSize)
        {
            strm.data = new byte[size];
            strm.bufferSize = size;
        }
        try
        {
            strm.is.read(strm.data, 0, size);
            strm.position += size;
        }
        catch (IOException e)
        {
        }
    }

    public long seekFileAndroid(NvAPKFile strm, int offset)
    {
        // Need to take care to seek the correct amount of bytes,
        // and to not end up in an infinite loop while doing so.
        // http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=4254082
        final int MAX_ITERATION = 128;
        long totalSkip = 0;
        int count = MAX_ITERATION;

        long skippedBytes = 0;

        try
        {
            strm.is.reset();
            while (offset > 0 && count > 0)
            {
                try
                {
                    skippedBytes = strm.is.skip(offset);
                }
                catch (IOException e)
                {
                    e.printStackTrace();
                }

                totalSkip += skippedBytes;
                offset -= skippedBytes;
                count--;
            }
        }
        catch (IOException e)
        {
        }

        strm.position = (int) totalSkip;
        return totalSkip;
    }

    public void closeFileAndroid(NvAPKFile strm)
    {
        try
        {
            strm.is.close();
        }
        catch (IOException e)
        {
        }
        strm.data = new byte[0];
        strm.is = null;
    }
}
