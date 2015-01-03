package com.minres.scviewer.database;

import java.io.File;

public interface ITransactionDbFactory {
	
	ITrDb createDatabase(File file);

}
