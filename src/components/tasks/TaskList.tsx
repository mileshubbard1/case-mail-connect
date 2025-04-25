
import { useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Checkbox } from "@/components/ui/checkbox";
import { Badge } from "@/components/ui/badge";
import { Calendar, Mail } from "lucide-react";
import type { Task } from "@/types/calendar";

// Mock tasks data combining calendar events and emails
const mockTasks: Task[] = [
  {
    id: "1",
    title: "Prepare for Smith v. Jones Hearing",
    source: "calendar",
    completed: false,
    dueDate: new Date("2025-04-20"),
    case: "Smith v. Jones",
    sourceId: "1"
  },
  {
    id: "2",
    title: "Review Pre-hearing Documentation",
    source: "email",
    completed: false,
    dueDate: new Date("2025-04-16"),
    case: "Smith v. Jones",
    sourceId: "e2"
  },
  {
    id: "3",
    title: "Client Meeting - Brighton Estates",
    source: "calendar",
    completed: false,
    dueDate: new Date("2025-04-18"),
    case: "Brighton Estates",
    sourceId: "2"
  }
];

export function TaskList() {
  const [tasks, setTasks] = useState<Task[]>(mockTasks);

  const toggleTask = (taskId: string) => {
    setTasks(tasks.map(task => 
      task.id === taskId ? { ...task, completed: !task.completed } : task
    ));
  };

  return (
    <Card>
      <CardHeader>
        <CardTitle>Today's Tasks</CardTitle>
      </CardHeader>
      <CardContent>
        <div className="space-y-4">
          {tasks.map(task => (
            <div
              key={task.id}
              className="flex items-center space-x-4 p-2 rounded-lg hover:bg-muted/50 transition-colors"
            >
              <Checkbox
                checked={task.completed}
                onCheckedChange={() => toggleTask(task.id)}
              />
              <div className="flex-1 min-w-0">
                <p className={`text-sm font-medium ${task.completed ? 'line-through text-muted-foreground' : ''}`}>
                  {task.title}
                </p>
                <div className="flex items-center gap-2 mt-1">
                  {task.case && (
                    <Badge variant="outline" className="text-xs">
                      {task.case}
                    </Badge>
                  )}
                  <span className="flex items-center text-xs text-muted-foreground gap-1">
                    {task.source === 'calendar' ? (
                      <Calendar className="h-3 w-3" />
                    ) : (
                      <Mail className="h-3 w-3" />
                    )}
                    {task.source === 'calendar' ? 'Calendar Event' : 'Email Task'}
                  </span>
                </div>
              </div>
              <span className="text-xs text-muted-foreground whitespace-nowrap">
                Due {task.dueDate.toLocaleDateString()}
              </span>
            </div>
          ))}
        </div>
      </CardContent>
    </Card>
  );
}
