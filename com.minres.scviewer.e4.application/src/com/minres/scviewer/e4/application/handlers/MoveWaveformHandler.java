 
package com.minres.scviewer.e4.application.handlers;

import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.CanExecute;
import org.eclipse.e4.core.di.annotations.Execute;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.workbench.modeling.EPartService;
import org.eclipse.e4.ui.workbench.modeling.ESelectionService;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.e4.application.parts.WaveformViewerPart;

public class MoveWaveformHandler {

	static final String PARAMETER_ID="com.minres.scviewer.e4.application.command.movewaveformupCommand.parameter.dir";

	@CanExecute
	public Boolean canExecute(ESelectionService selectionService){
		Object o = selectionService.getSelection();
		return o instanceof IWaveform<?> || o instanceof ITx;
	}
	
	@Execute
	public void execute(@Named(PARAMETER_ID) String param, EPartService partService) {
		MPart part = partService.getActivePart();
		Object obj = part.getObject();
		if(obj instanceof WaveformViewerPart){
			if("up".equalsIgnoreCase(param))
				((WaveformViewerPart)obj).moveSelected(-1);
			else if("down".equalsIgnoreCase(param))
				((WaveformViewerPart)obj).moveSelected(1);
		}
	}
		
}