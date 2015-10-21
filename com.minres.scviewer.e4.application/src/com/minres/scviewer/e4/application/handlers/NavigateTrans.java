
package com.minres.scviewer.e4.application.handlers;

import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.CanExecute;
import org.eclipse.e4.core.di.annotations.Execute;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.workbench.modeling.EPartService;
import org.eclipse.e4.ui.workbench.modeling.ESelectionService;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.swt.GotoDirection;
import com.minres.scviewer.e4.application.parts.WaveformViewerPart;

public class NavigateTrans {

	final static String PARAMTER_ID="com.minres.scviewer.e4.application.command.navigateTransCommand.parameter.dir";
	
	@CanExecute
	public Boolean canExecute(ESelectionService selectionService){
		Object o = selectionService.getSelection();
		return o instanceof ITx;
	}
	
	@Execute
	public void execute(@Named(PARAMTER_ID) String param, EPartService partService) {
		MPart part = partService.getActivePart();
		Object obj = part.getObject();
		if(obj instanceof WaveformViewerPart){
			if("next".equalsIgnoreCase(param))
				((WaveformViewerPart)obj).moveSelection(GotoDirection.NEXT);
			else if("prev".equalsIgnoreCase(param))
				((WaveformViewerPart)obj).moveSelection(GotoDirection.PREV);
		}
	}
}