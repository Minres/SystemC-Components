 
package com.minres.scviewer.e4.application.handlers;

import org.eclipse.e4.core.di.annotations.CanExecute;
import org.eclipse.e4.core.di.annotations.Execute;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.workbench.modeling.ESelectionService;

import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.e4.application.parts.WaveformViewerPart;

public class DeleteWaveformHandler {
	
	@CanExecute
	public Boolean canExecute(ESelectionService selectionService){
		Object o = selectionService.getSelection();
		return o instanceof IWaveform<?>;
	}
	
	@Execute
	public void execute(ESelectionService selectionService, MPart activePart) {
		Object o = activePart.getObject();
		Object sel = selectionService.getSelection();
		if(o instanceof WaveformViewerPart && sel instanceof IWaveform<?>){
			((WaveformViewerPart)o).removeStreamFromList((IWaveform<?>) sel);
		}	
	}
		
}