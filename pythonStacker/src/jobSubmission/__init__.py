

def batching(commandlist: list, batchsize: int, basecommands: str = ""):
    '''
    Commandlist: set of commands to batch in batches of batchsize.
    Batchsize: how many commands to run from commandlist in each iteration
    Basecommands: commands to run before any command in commandlist. e.g. to set environment variables or others. Ideally this is integrated in condorTools.py in the future.
    '''
    batched = []
    i = 0
    while i * batchsize < len(commandlist):
        if basecommands == "":
            batch = []
        else:
            batch = [basecommands]

        if (i + 1) * batchsize > len(commandlist):
            batch.extend(commandlist[i * batchsize:])
        else:
            batch.extend(commandlist[i * batchsize:(i+1) * batchsize])
        i += 1
        batched.append(batch)
    return batched
