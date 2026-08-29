# 4d-plugin-structure-access

**Create Elements from XML** is a 4D plugin command that allows a 4D application to create database structure elements from an XML definition.

The underlying 4D Plugin API operation supports creating:

- Tables
- Fields
- Indexes
- Relations

from an XML definition.

The plugin is intentionally small: it passes the XML supplied by the 4D developer to 4D's native `Create Elements From XML Definition` API.

### What this plugin does

```text
4D XML string
	  │
	  ▼
Create Elements from XML
	  │
	  ▼
4D database structure
```

The plugin does **not** parse or transform the XML itself. The XML is passed directly to the 4D API.

---

# Command

## CREATE ELEMENTS FROM XML

Creates database structure elements using an XML definition.

### Syntax

```4d
CREATE ELEMENTS FROM XML(xmlDefinition)
```

### Parameter

| Parameter | Type | Description |
|---|---|---|
| `xmlDefinition` | Text | XML definition describing the database elements to create |

The parameter must contain the XML definition expected by 4D's database-structure XML mechanism.

The plugin API accepts the XML as a 4D Unicode string (`PA_Unistring`).

### Return value

The command does not return a value.

It is an action command: the supplied XML is passed to 4D and the requested structure elements are created.

---

# Basic Usage

The simplest usage is to place the XML definition in a 4D text variable and pass it to the command.

```4d
var $xml : Text

$xml := "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + \
"<database>" + \
	"..." + \
"</database>"

CREATE ELEMENTS FROM XML($xml)
```

The exact XML structure must follow the XML definition format supported by the 4D version being used.

The plugin does not provide a second XML syntax or a plugin-specific XML dialect.

---

# Using an XML File

A practical use case is loading an XML definition from a file and passing the resulting text to the command.

For example:

```4d
var $xml : Text
var $path : 4D.File
var $result : Object

$path := File("/path/to/database-definition.xml")

If ($path.exists)
	$xml := $path.getText()

	CREATE ELEMENTS FROM XML($xml)
End if
```

The important part is that `$xml` contains the XML definition when the command is executed.

The plugin itself does not open the file. File reading is performed by 4D.

---

# Building XML Dynamically

The XML can also be constructed by 4D code.

For example:

```4d
var $xml : Text
var $tableName : Text
var $fieldName : Text

$tableName := "Customers"
$fieldName := "Name"

$xml := "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + \
"<database>" + \
	"<table name=\"" + $tableName + "\">" + \
		"<field name=\"" + $fieldName + "\"/>" + \
	"</table>" + \
"</database>"

CREATE ELEMENTS FROM XML($xml)
```

When constructing XML dynamically, make sure that values inserted into XML are properly XML-escaped.

For example, a value containing:

```text
&
<
>
"
'
```

may need escaping depending on where it occurs in the XML.

For complex definitions, it is generally preferable to construct the XML using an appropriate 4D XML facility rather than concatenating large strings manually.

---

# Creating Multiple Elements

One XML definition can describe multiple database elements.

The underlying API specifically describes the operation as supporting tables, fields, indexes, and/or relations.

Conceptually:

```4d
var $xml : Text

$xml := $definitionContainingMultipleElements

CREATE ELEMENTS FROM XML($xml)
```

The plugin does not need to be called once per table or field.

---

# Example: Loading a Definition

A useful pattern for development or deployment scripts is:

```4d
var $xml : Text
var $file : 4D.File

$file := File("/Definitions/customer_database.xml")

If (Not($file.exists))
	ALERT("Database definition file not found.")
Else
	$xml := $file.getText()

	CREATE ELEMENTS FROM XML($xml)
End if
```

This keeps the XML definition outside the 4D method and makes the definition easier to version and maintain.

---

# Example: Conditional Structure Creation

You can use normal 4D logic around the command.

```4d
var $xml : Text

If (Confirm("Create the database structure?")=1)

	$xml := $definition

	CREATE ELEMENTS FROM XML($xml)

	ALERT("Database structure creation requested.")
End if
```

The plugin command itself does not provide a confirmation dialog.

---

# Example: Deployment Method

A deployment method could look like:

```4d
var $xml : Text

$xml := File("/Deployment/database.xml").getText()

If ($xml#"")
	CREATE ELEMENTS FROM XML($xml)
End if
```

This allows the XML definition to be maintained separately from the deployment method.

---

# XML Input

The plugin passes the supplied XML directly to 4D.

The underlying implementation effectively performs:

```text
XML supplied by 4D
		↓
4D Plugin API
		↓
4D Create Elements operation
```

The API function is declared as:

```text
PA_CreateElementsFromXMLDefinition(xmlDefinition)
```

and accepts a pointer to a Unicode 4D string.

The API implementation passes the Unicode string to the 4D engine without making the plugin perform XML parsing itself.

Therefore, the plugin should be considered a **thin 4D wrapper**, rather than an XML-processing library.

---

# Error Handling

The command does not return an error value.

The underlying Plugin API records the error status associated with an API call. The API provides:

```4d
PA_GetLastError()
```

for obtaining the last error returned by an API call.

For normal 4D use, the important point is that an unsuccessful structure operation should not be interpreted as a successful modification merely because the plugin command itself has no return value.

If your application requires detailed validation of a database definition, validate the XML and the intended database changes before invoking the command.

---

# Important: This Command Changes Database Structure

This is not a data-manipulation command.

It operates on the **database structure**.

The API documentation explicitly places this function in the section for:

- Tables
- Fields
- Indexes
- Relations

Accordingly, treat calls to this command as structural/deployment operations rather than as ordinary record-processing operations.

### Recommended practice

Do not casually execute dynamically generated XML against a production database.

Before using the command in production:

1. Verify the XML definition.
2. Test it against a development database.
3. Verify the resulting tables and fields.
4. Verify indexes and relations.
5. Back up the database when appropriate.
6. Use controlled deployment procedures.

---

# Large XML Definitions

The plugin does not impose its own XML size limit.

However, creating a large database structure can naturally require substantial processing time and memory inside 4D.

For this reason, avoid repeatedly submitting the same large definition unnecessarily.

Prefer:

```4d
If (Not($alreadyDeployed))
	CREATE ELEMENTS FROM XML($xml)
End if
```

over repeatedly executing the operation on every startup or every record-processing cycle.

The plugin itself does not contain a loop around the operation; each invocation makes one call to the underlying 4D operation.

---

# Repeated Calls

The command can be called from normal 4D code, but callers should avoid unnecessary repetition.

For example, avoid patterns such as:

```4d
For ($i; 1; 10000)
	CREATE ELEMENTS FROM XML($xml)
End for
```

unless there is a specific reason to do so.

Database structure operations are fundamentally different from normal record operations and should generally be performed as controlled deployment/setup operations.

---

# Server / Client Considerations

The supplied Plugin API supports both 4D Client and 4D Server environments and exposes functions for determining the current environment, including:

```text
PA_Is4DClient()
PA_Is4DServer()
PA_Is4DMono()
```

as well as web-process detection.

This particular plugin does not add its own client/server synchronization layer.

Therefore, when using the command in a client/server application, structure-changing operations should be deliberately placed in the appropriate deployment/administration context rather than being triggered indiscriminately by ordinary client code.

---

# Threading and Processes

The plugin does not create its own worker threads or background processes.

The supplied Plugin API contains facilities for creating processes, yielding, freezing/unfreezing processes, and running code in the main process, but this plugin does not use those facilities.

Consequently, the command should be treated as a synchronous operation from the calling 4D method's perspective.

Do not assume that starting the command makes the structure operation run asynchronously.

---

# Performance Considerations

For normal-sized definitions, the plugin adds essentially no processing layer of its own.

The processing path is:

```text
4D method
   ↓
plugin command
   ↓
4D structure API
```

The XML parsing and database-structure work are performed by 4D's underlying implementation.

Therefore, if a particular XML definition takes a long time to process, the likely source of the processing time is the size/complexity of the structure operation rather than plugin-side XML processing.

### Avoid

```4d
// Repeated structural operations during normal application processing
For ($i; 1; Records in selection([MyTable]))
	CREATE ELEMENTS FROM XML($xml)
End for
```

### Prefer

```4d
// One controlled deployment/setup operation
CREATE ELEMENTS FROM XML($xml)
```

---

# Unicode and Text

The underlying parameter is a 4D Unicode string.

The Plugin API exposes `PA_Unistring` for Unicode strings and provides functions such as `PA_GetUnistring`, `PA_GetUnistringLength`, `PA_CreateUnistring`, and `PA_DisposeUnistring`.

For the 4D developer this means the command should normally be used with a 4D **Text** value containing the XML.

You do not need to convert the XML to an old-style byte string before calling the command.

---

# Recommended Deployment Pattern

For a production application, a simple deployment method can be organized like this:

```4d
// DeployDatabaseStructure
var $file : 4D.File
var $xml : Text

$file := File("/Deployment/database.xml")

If (Not($file.exists))
	ALERT("Database definition not found.")
	Return
End if

$xml := $file.getText()

If ($xml="")
	ALERT("Database definition is empty.")
	Return
End if

CREATE ELEMENTS FROM XML($xml)
```

A more complete deployment process can add application-specific checks:

```4d
// DeployDatabaseStructure
var $file : 4D.File
var $xml : Text

$file := File("/Deployment/database.xml")

If (Not($file.exists))
	ALERT("Database definition not found.")
	Return
End if

$xml := $file.getText()

If ($xml="")
	ALERT("Database definition is empty.")
	Return
End if

// Optional application-specific checks go here.

// Perform the structural operation.
CREATE ELEMENTS FROM XML($xml)
```

---

# Best Practices

### Do

- Keep database definitions under version control.
- Test XML definitions on a development database first.
- Use the command for controlled structure/deployment operations.
- Keep large XML definitions in files rather than enormous 4D methods when practical.
- Avoid repeatedly executing the same structure definition.
- Validate dynamically generated XML before submitting it.
- Back up important databases before structural deployment.
- Clearly separate deployment/setup code from normal record-processing code.

### Avoid

- Executing arbitrary XML supplied by untrusted users.
- Running structure creation inside high-frequency record loops.
- Rebuilding the database structure unnecessarily.
- Assuming that the command is asynchronous.
- Treating completion of the 4D method as proof that every requested structural change succeeded.
- Modifying or disposing the XML parameter specifically for the plugin.

---

# Security Considerations

The XML passed to this command is effectively a request to modify database structure.

Therefore, access to code that invokes this command should be restricted appropriately.

For example, avoid designs where ordinary users can supply arbitrary XML and have it passed directly to:

```4d
CREATE ELEMENTS FROM XML($userSuppliedXML)
```

unless that behavior is explicitly intended and controlled.

A safer architecture is:

```text
Administrator / deployment system
			 │
			 ▼
	 approved XML definition
			 │
			 ▼
 CREATE ELEMENTS FROM XML
```

rather than:

```text
untrusted user input
		│
		▼
CREATE ELEMENTS FROM XML
```

---

# Summary

**CREATE ELEMENTS FROM XML** provides a simple 4D-facing interface for submitting an XML database definition to 4D.

### Syntax

```4d
CREATE ELEMENTS FROM XML(xmlDefinition)
```

### Input

A 4D Text value containing the XML database definition.

### Creates

Depending on the XML definition, the underlying 4D operation supports:

- Tables
- Fields
- Indexes
- Relations

### Output

None.

### Typical use

```4d
var $xml : Text

$xml := File("/Deployment/database.xml").getText()

CREATE ELEMENTS FROM XML($xml)
```

### Intended use

The command is best suited to **database structure creation and deployment**, not ordinary application record processing.

The plugin itself performs no XML transformation and no additional database processing; it forwards the supplied XML to the corresponding 4D Plugin API operation.