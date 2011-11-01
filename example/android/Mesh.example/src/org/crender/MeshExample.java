package org.crender;

import android.os.Bundle;
import com.nvidia.devtech.*;  

public class MeshExample extends NvEventQueueActivity {

	public void onCreate(Bundle savedInstanceState)
    {
		supportPauseResume = true;
        super.onCreate(savedInstanceState);
    }

    static
    {
        System.loadLibrary("crender-mesh-example");
    }
    
}
